//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "frame_mod_browser.h"
#include <GlobalObjects.h>
#include <tab_mod_browser.h>
#include <tab_mod_presets.h>
#include <tab_mod_options.h>
#include <ext_GlobalObjects.h>
#include "toolbox.h"

frame_mod_browser::frame_mod_browser(std::string folder_){

  ext_GlobalObjects::setCurrentFrameModBrowserPtr(this);

  std::string game_path = GlobalObjects::get_mod_browser().get_current_directory() + "/" + folder_;

  _icon_ = nullptr;

  this->setTitle(folder_);
  _titleid_ = toolbox::recursive_search_for_subfolder_name_like_tid(game_path);
  _icon_ = GlobalObjects::get_mod_browser().get_folder_icon_from_titleid(_titleid_);
  if(_icon_ != nullptr){
    this->setIcon(_icon_, 0x20000);
  }
  this->setFooterText("SimpleModManager");

  if( GlobalObjects::get_mod_browser().change_directory(game_path) ){

    GlobalObjects::get_mod_browser().get_mod_manager().set_current_mods_folder(game_path);
    GlobalObjects::get_mod_browser().get_mods_preseter().read_parameter_file(game_path);

    auto* parametersTabList = new brls::List();
    GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name();
    auto* presetParameter = new brls::ListItem("Config preset", "", "");
    presetParameter->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
    parametersTabList->addView(presetParameter);

    _tabModBrowser_ = new tab_mod_browser();
    _tabModPresets_ = new tab_mod_presets();
    _tabModOptions_ = new tab_mod_options();

    _tabModOptions_->initialize();

    this->addTab("Mod Browser", _tabModBrowser_);
    this->addSeparator();
    this->addTab("Mod Presets", _tabModPresets_);
    this->addTab("Options", _tabModOptions_);

  }
  else{
    auto* list = new brls::List();
    brls::Logger::error("Can't open: %s", game_path.c_str());
    auto* item = new brls::ListItem("Error: Can't open " + game_path , "", "");
    list->addView(item);
    this->addTab("Mod Browser", list);
  }

}

bool frame_mod_browser::onCancel() {

  // Go back to sidebar
  auto* lastFocus = brls::Application::getCurrentFocus();
  brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, false);

  // If the sidebar was already there, the focus has not changed
  if(lastFocus == brls::Application::getCurrentFocus()){
    brls::Logger::debug("go_back");
    GlobalObjects::get_mod_browser().go_back();
    brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  }
  return true;

}

uint8_t *frame_mod_browser::getIcon() {
  return _icon_;
}

std::string frame_mod_browser::getTitleid() {
  return _titleid_;
}
