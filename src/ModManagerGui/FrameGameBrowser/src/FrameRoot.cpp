//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "FrameRoot.h"

#include <TabGames.h>
#include <TabAbout.h>
#include <TabGeneralSettings.h>

#include <GlobalObjects.h>
#include "Toolbox.h"

#include "GenericToolbox.h"
#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[FrameRoot]");
});


FrameRoot::FrameRoot() {
  LogWarning << "Build root frame..." << std::endl;

  this->setTitle("SimpleModManager");
  this->setFooterText( "v" + Toolbox::getAppVersion() );
  this->setIcon("romfs:/images/icon.jpg");
  this->addTab( "Game Browser", new TabGames(this) );
  this->addSeparator();
  this->addTab( "Settings", new TabGeneralSettings(this) );
  this->addTab( "About", new TabAbout() );

  LogInfo << "Root frame built." << std::endl;
}

bool FrameRoot::onCancel() {
  // fetch the current focus
  auto* lastFocus = brls::Application::getCurrentFocus();

  // perform the cancel
  bool onCancel = TabFrame::onCancel();

  // if the focus is the same, then quit the app
  if( lastFocus == brls::Application::getCurrentFocus() ){ brls::Application::quit(); }

  return onCancel;
}

const GuiModManager &FrameRoot::getGuiModManager() const {
  return _guiModManager_;
}
GuiModManager &FrameRoot::getGuiModManager() {
  return _guiModManager_;
}
