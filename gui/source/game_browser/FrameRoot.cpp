//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "FrameRoot.h"
#include <GlobalObjects.h>
#include <TabGames.h>
#include <TabGeneralSettings.h>
#include <TabAbout.h>

#include "GenericToolbox.h"
#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[FrameRoot]");
});

FrameRoot::FrameRoot() {
  LogInfo << __METHOD_NAME__ << std::endl;

  this->setTitle("SimpleModManager");
  this->setFooterText(GlobalObjects::_version_str_);
  this->setIcon("romfs:/images/icon.jpg");
  this->addTab("Game Browser", new TabGames());
  this->addSeparator();
  this->addTab("Settings", new TabGeneralSettings());
  this->addTab("About", new TabAbout());

  LogInfo << __METHOD_NAME__ << std::endl;
}

bool FrameRoot::onCancel() {

  auto* lastFocus = brls::Application::getCurrentFocus();

  bool onCancel = TabFrame::onCancel();

  if(lastFocus == brls::Application::getCurrentFocus()){
    brls::Application::quit();
  }

  return onCancel;
}
