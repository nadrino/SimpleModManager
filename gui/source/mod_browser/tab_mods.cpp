//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "tab_mods.h"
#include <GlobalObjects.h>

tab_mods::tab_mods(std::string folder_) {

  auto* modsBrowserTabList = new brls::List();

  this->setTitle(folder_);
  auto* icon = GlobalObjects::get_mod_browser().get_folder_icon(folder_);
  if(icon != nullptr){
    this->setIcon(icon, 0x20000);
  }
  this->setFooterText("SimpleModManager");

  // Setup the list
  std::string game_path = GlobalObjects::get_mod_browser().get_current_directory() + "/" + folder_;
  if(GlobalObjects::get_mod_browser().change_directory(game_path)){
    GlobalObjects::get_mod_browser().get_mod_manager().set_current_mods_folder(game_path);
    GlobalObjects::get_mod_browser().check_mods_status();
    GlobalObjects::get_mod_browser().get_mods_preseter().read_parameter_file(game_path);
    auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
    for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
      std::string selected_folder = mod_folders_list[i_folder];
      auto* item = new brls::ListItem(selected_folder, "", "");
      item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_folder));
      item->getClickEvent()->subscribe([this, item, selected_folder](View* view) {
        brls::Logger::debug("Applying mod: %s", selected_folder.c_str());
        // apply mode
        std::string dialogTitle = "Applying: " + selected_folder + "\n";

        auto* frame = new brls::AppletFrame(false, false);
        frame->setHeight(600);

        auto* progressBar = new brls::ProgressDisplay();
        frame->setContentView(progressBar);

        brls::PopupFrame::open("Popup title", BOREALIS_ASSET("icon/borealis.jpg"), frame, "Subtitle left", "Subtitle right");


        item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_folder));
      });
      item->updateActionHint(brls::Key::A, "Apply");

      modsBrowserTabList->addView(item);
      _mods_list_.emplace_back(item);

    }
  }
  else{
    brls::Logger::error("Can't open: %s", game_path.c_str());
    auto* item = new brls::ListItem("Error: Can't open " + game_path , "", "");
    modsBrowserTabList->addView(item);
  }

  auto* parametersTabList = new brls::List();
  GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name();
  auto* presetParameter = new brls::ListItem("Config preset", "", "");
  presetParameter->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
  parametersTabList->addView(presetParameter);

  auto* modsPresetsTabList = new brls::List();


  this->addTab("Mod Browser", modsBrowserTabList);
  this->addSeparator();
  this->addTab("Mod Presets", modsPresetsTabList);
  this->addTab("Options", parametersTabList);

}

bool tab_mods::onCancel() {

  GlobalObjects::get_mod_browser().go_back();
  brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  return true;

}
