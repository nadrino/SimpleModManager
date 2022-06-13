//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "FrameModBrowser.h"
#include <GlobalObjects.h>
#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModPresets.h>
#include <TabModOptions.h>
#include <ext_GlobalObjects.h>
#include "toolbox.h"

#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[FrameModBrowser]");
});

FrameModBrowser::FrameModBrowser(std::string folder_){

  ext_GlobalObjects::setCurrentFrameModBrowserPtr(this);

  std::string game_path = GlobalObjects::getModBrowser().get_current_directory() + "/" + folder_;

  _icon_ = nullptr;

  this->setTitle(folder_);
  _titleid_ = toolbox::recursive_search_for_subfolder_name_like_tid(game_path);
  _icon_ = GlobalObjects::getModBrowser().get_folder_icon_from_titleid(_titleid_);
  if(_icon_ != nullptr){
    this->setIcon(_icon_, 0x20000);
  }
  this->setFooterText("SimpleModManager");

  if(GlobalObjects::getModBrowser().change_directory(game_path) ){

    GlobalObjects::getModBrowser().get_mod_manager().set_current_mods_folder(game_path);
    GlobalObjects::getModBrowser().get_mods_preseter().read_parameter_file(game_path);

    auto* parametersTabList = new brls::List();
    GlobalObjects::getModBrowser().get_parameters_handler().get_current_config_preset_name();
    auto* presetParameter = new brls::ListItem("Config preset", "", "");
    presetParameter->setValue(GlobalObjects::getModBrowser().get_parameters_handler().get_current_config_preset_name());
    parametersTabList->addView(presetParameter);

    _tabModBrowser_ = new TabModBrowser();
    _tabModPresets_ = new TabModPresets();
    _tabModOptions_ = new TabModOptions();
    _tabModPlugins_ = new TabModPlugins();

    _tabModOptions_->initialize();

    this->addTab("Mod Browser", _tabModBrowser_);
    this->addSeparator();
    this->addTab("Mod Presets", _tabModPresets_);
    this->addTab("Options", _tabModOptions_);
    this->addTab("Plugins", _tabModPlugins_);

  }
  else{
    auto* list = new brls::List();
    LogError("Can't open: %s", game_path.c_str());
    auto* item = new brls::ListItem("Error: Can't open " + game_path , "", "");
    list->addView(item);
    this->addTab("Mod Browser", list);
  }

}

bool FrameModBrowser::onCancel() {

  // Go back to sidebar
  auto* lastFocus = brls::Application::getCurrentFocus();
  brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, false);

  // If the sidebar was already there, the focus has not changed
  if(lastFocus == brls::Application::getCurrentFocus()){
    LogDebug("Back on games screen...");
    GlobalObjects::getModBrowser().go_back();
    brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  }
  return true;

}

uint8_t *FrameModBrowser::getIcon() {
  return _icon_;
}

std::string FrameModBrowser::getTitleid() {
  return _titleid_;
}
