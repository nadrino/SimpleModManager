//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModOptions.h"
#include <GlobalObjects.h>
#include <ext_GlobalObjects.h>
#include <toolbox.h>

TabModOptions::TabModOptions() {

  frameSkipCount = 0;
  doUpdateModsStatus = false;
  _preSelection_ = 0;
  _inheritedTitle_ = "Inherited from the main menu";
  _itemFolderInstallPreset_ = nullptr;

}

void TabModOptions::buildFolderInstallPresetItem() {

  _itemFolderInstallPreset_ = new brls::ListItem(
    "\uE255 Attribute a config preset",
    "Specify from which install folder mods from this subfolder (game) will be installed.\n",
    ""
  );
  _itemFolderInstallPreset_->setValue(_inheritedTitle_);

  // Find the current selection
  std::string folderConfigFilePath = GlobalObjects::get_mod_browser().get_current_directory() + "/this_folder_config.txt";
  if(toolbox::do_path_is_file(folderConfigFilePath)){
    _preSelection_ = 1 + GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_id();
    _itemFolderInstallPreset_->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
  }

  // On click : show scrolling up menu
  _itemFolderInstallPreset_->getClickEvent()->subscribe([this](View* view) {

    // build the choice list
    std::vector<std::string> config_presets_list;
    config_presets_list.emplace_back(this->_inheritedTitle_);
    for(const auto& preset_name: GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list()){
      config_presets_list.emplace_back(preset_name);
    }

    // function that will set the config preset from the Dropdown menu selection (int result)
    brls::ValueSelectedEvent::Callback valueCallback = [this, valueCallback](int result) {
      if (result == -1)
        return;

      if(result == this->_preSelection_){
        brls::Application::popView();
        return;
      }

      this->_preSelection_ = result;

      // overwriting
      std::string this_folder_config_file_path = GlobalObjects::get_mod_browser().get_current_directory() + "/this_folder_config.txt";
      toolbox::delete_file(this_folder_config_file_path);
      if(result > 0){
        // then a preset has been specified
        toolbox::dump_string_in_file(
          GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list()[result-1],
          this_folder_config_file_path
        );
        GlobalObjects::get_mod_browser().change_config_preset(
          GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list()[result-1]
        );
        this->_itemFolderInstallPreset_->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
      }
      else{
        // restore the config preset from the main menu
        GlobalObjects::get_mod_browser().change_config_preset(
          GlobalObjects::get_mod_browser().get_main_config_preset()
        );
        this->_itemFolderInstallPreset_->setValue(_inheritedTitle_);
      }

      if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
        GuiModManager::setOnCallBackFunction([](){brls::Application::popView(brls::ViewAnimation::FADE);});
        GlobalObjects::get_mod_browser().get_mod_manager().reset_all_mods_cache_status();
//        ext_GlobalObjects::getCurrentTabModBrowserPtr()->refreshModsStatus();
      }

    }; // Callback sequence

    brls::Dropdown::open(
      "Please select the config preset you want for this folder",
      config_presets_list,
      valueCallback,
      this->_preSelection_,
      true
    );

  });

}
void TabModOptions::buildResetModsCacheItem() {

  _itemResetModsCache_ = new brls::ListItem(
    "\uE877 Recheck all mods",
    "\tThis option resets all mods cache status and recheck if each files is properly applied.\n"
              "\tWhen files where managed without SimpleModManager, the displayed mod status can be wrong. "
              "This typically happens when you modified some mod files, or other programs override some of the applied files.",
    ""
  );

  _itemResetModsCache_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("Do you want to reset mods cache status and recheck all mod files ?");

    dialog->addButton("Yes", [this, dialog](brls::View* view) {
      if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
        GuiModManager::setOnCallBackFunction([dialog](){dialog->close();});
        GlobalObjects::get_mod_browser().get_mod_manager().reset_all_mods_cache_status();
        ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_check_all_mods();
//        ext_GlobalObjects::getCurrentTabModBrowserPtr()->refreshModsStatus();
      }
    });
    dialog->addButton("No", [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildDisableAllMods() {

  _itemDisableAllMods_ = new brls::ListItem(
    "\uE872 Disable all mods",
    "This option will remove all installed mods files.\n"
              "Note: to be deleted, installed mod files need to be identical to the one present in this folder.",
    ""
  );

  _itemDisableAllMods_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("Do you want to disable all mods ?");

    dialog->addButton("Yes", [dialog](brls::View* view) {
      if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
        GuiModManager::setOnCallBackFunction([dialog](){dialog->close();});
        ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_remove_all_mods();
      }
    });
    dialog->addButton("No", [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildGameIdentificationItem(){

  _itemGameIdentification_ = new brls::ListItem(
    "Associated TitleID",
    "",
    "Current value :"
  );
  if(ext_GlobalObjects::getCurrentFrameModBrowserPtr()->getIcon() != nullptr){
    _itemGameIdentification_->setValue(ext_GlobalObjects::getCurrentFrameModBrowserPtr()->getTitleid());
    _itemGameIdentification_->setThumbnail(
      ext_GlobalObjects::getCurrentFrameModBrowserPtr()->getIcon(),
      0x20000
    );
  }
  else{
    _itemGameIdentification_->setValue("No TitleID Candidate");
  }

}

void TabModOptions::initialize() {

  this->buildFolderInstallPresetItem();
  this->buildResetModsCacheItem();
  this->buildDisableAllMods();
  this->buildGameIdentificationItem();

  // finally add to view
  this->addView(_itemResetModsCache_);
  this->addView(_itemFolderInstallPreset_);
  this->addView(_itemDisableAllMods_);
  this->addView(_itemGameIdentification_);

}

void TabModOptions::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {
  ScrollView::draw(vg, x, y, width, height, style, ctx);

}




