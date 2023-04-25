//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModOptions.h"
#include "FrameModBrowser.h"

#include <GlobalObjects.h>

#include "GenericToolbox.h"


TabModOptions::TabModOptions(FrameModBrowser* owner_) : _owner_(owner_) {  }

void TabModOptions::buildFolderInstallPresetItem() {

  _itemFolderInstallPreset_ = new brls::ListItem(
    "\uE255 Attribute a config preset",
    "Specify from which install folder mods from this subfolder (game) will be installed.\n",
    ""
  );
  _itemFolderInstallPreset_->setValue(_inheritedTitle_);


  // Find the current selection
  std::string folderConfigFilePath = this->getModManager().getGameFolderPath() + "/this_folder_config.txt";
  if( GenericToolbox::doesPathIsFile(folderConfigFilePath) ){
    _preSelection_ = 1 + this->getModManager().getConfig().selectedPresetIndex;
    _itemFolderInstallPreset_->setValue( this->getModManager().getConfig().getCurrentPresetName() );
  }

  // On click : show scrolling up menu
  _itemFolderInstallPreset_->getClickEvent()->subscribe([this](View* view) {

    // build the choice list
    std::vector<std::string> config_presets_list;
    config_presets_list.emplace_back(_inheritedTitle_);
    for( auto& preset: this->getModManager().getConfig().presetList ){
      config_presets_list.emplace_back( preset.name );
    }

    // function that will set the config preset from the Dropdown menu selection (int result)
    brls::ValueSelectedEvent::Callback valueCallback = [&](int result) {
      if (result == -1) return;

      if(result == this->_preSelection_){
        brls::Application::popView();
        return;
      }

      this->_preSelection_ = result;

      // overwriting
      std::string thisFolderConfigFilePath = this->getModManager().getGameFolderPath() + "/this_folder_config.txt";
      GenericToolbox::deleteFile(thisFolderConfigFilePath);
      if(result > 0){
        this->getModManager().getConfig().setSelectedPresetIndex(result - 1);
        // then a preset has been specified
        GenericToolbox::dumpStringInFile(
            thisFolderConfigFilePath,
            this->getModManager().getConfig().getCurrentPresetName()
        );
        _itemFolderInstallPreset_->setValue( this->getModManager().getConfig().getCurrentPresetName() );
      }
      else{
        // restore the config preset from the main menu
        _itemFolderInstallPreset_->setValue(_inheritedTitle_);
      }

      this->getModManager().reloadModStatusCache();
    }; // Callback sequence

    brls::Dropdown::open(
      "Please select the config preset you want for this folder",
      config_presets_list, valueCallback,
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

    dialog->addButton("Yes", [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startCheckAllModsThread();
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

    dialog->addButton("Yes", [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startRemoveAllModsThread();
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

  if( _owner_->getIcon() != nullptr ){
    _itemGameIdentification_->setValue( _owner_->getTitleId() );
    _itemGameIdentification_->setThumbnail( _owner_->getIcon(), 0x20000 );
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

const ModManager &TabModOptions::getModManager() const {
  return _owner_->getGameBrowser().getModManager();
}
ModManager &TabModOptions::getModManager() {
  return _owner_->getGameBrowser().getModManager();
}




