//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "FrameModBrowser.h"

#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModPresets.h>
#include <TabModOptions.h>

#include "GenericToolbox.Switch.h"
#include "Logger.h"


LoggerInit([]{
  Logger::setUserHeaderStr("[FrameModBrowser]");
});


FrameModBrowser::FrameModBrowser(GuiModManager* guiModManagerPtr_) : _guiModManagerPtr_(guiModManagerPtr_) {

  // fetch game title
  this->setTitle( getGameBrowser().getModManager().getGameName() );

  std::string gamePath = getGameBrowser().getModManager().getGameFolderPath();

  _titleId_ = GenericToolbox::Switch::Utils::lookForTidInSubFolders( gamePath, 5);
  _icon_ = GenericToolbox::Switch::Utils::getIconFromTitleId( _titleId_ );
  if(_icon_ != nullptr){ this->setIcon(_icon_, 0x20000); }
  else{ this->setIcon("romfs:/images/icon_corner.png"); }

  this->setFooterText("SimpleModManager");


  if( not getGameBrowser().getModManager().getModList().empty() ){

    auto* parametersTabList = new brls::List();

    auto* presetParameter = new brls::ListItem("Config preset", "", "");
    presetParameter->setValue( getGameBrowser().getConfigHandler().getConfig().getCurrentPresetName() );
    parametersTabList->addView(presetParameter);

    _tabModBrowser_ = new TabModBrowser( this );
    _tabModPresets_ = new TabModPresets( this );
    _tabModOptions_ = new TabModOptions( this );
    _tabModPlugins_ = new TabModPlugins( this );

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
