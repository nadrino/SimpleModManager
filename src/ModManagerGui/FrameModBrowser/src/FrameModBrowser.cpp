//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "FrameModBrowser.h"

#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModPresets.h>
#include <TabModOptions.h>

#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"
#include "Logger.h"


LoggerInit([]{
  Logger::setUserHeaderStr("[FrameModBrowser]");
});

FrameModBrowser::FrameModBrowser(const std::string& folder_){

  this->setTitle(folder_);

  std::string gamePath = GlobalObjects::getModBrowser().get_current_directory() + "/" + folder_;
  _titleId_ = GenericToolbox::Switch::Utils::lookForTidInSubFolders(gamePath);
  _icon_ = GlobalObjects::getModBrowser().getFolderIcon(folder_);
  if(_icon_ != nullptr){ this->setIcon(_icon_, 0x20000); }
  this->setFooterText("SimpleModManager");

  if(GlobalObjects::getModBrowser().change_directory(gamePath) ){

    GlobalObjects::getModBrowser().getModManager().set_current_mods_folder(gamePath);
    GlobalObjects::getModBrowser().get_mods_preseter().read_parameter_file(gamePath);

    auto* parametersTabList = new brls::List();
    GlobalObjects::getModBrowser().get_parameters_handler().get_current_config_preset_name();
    auto* presetParameter = new brls::ListItem("Config preset", "", "");
    presetParameter->setValue(GlobalObjects::getModBrowser().get_parameters_handler().get_current_config_preset_name());
    parametersTabList->addView(presetParameter);

    _tabModBrowser_ = new TabModBrowser( this );
    _tabModPresets_ = new TabModPresets( this );
    _tabModOptions_ = new TabModOptions( this );
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
    LogError("Can't open: %s", gamePath.c_str());
    auto* item = new brls::ListItem("Error: Can't open " + gamePath , "", "");
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
    LogInfo("Back on games screen...");
    GlobalObjects::getModBrowser().go_back();
    brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  }
  return true;

}

uint8_t *FrameModBrowser::getIcon() {
  return _icon_;
}
std::string FrameModBrowser::getTitleId() {
  return _titleId_;
}
GuiModManager &FrameModBrowser::getModManager() {
  return _modManager_;
}
