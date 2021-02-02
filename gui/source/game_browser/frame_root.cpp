//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "frame_root.h"
#include <GlobalObjects.h>
#include <tab_games.h>
#include <tab_general_settings.h>
#include <tab_about.h>
#include <game_browser/tab_test.h>

frame_root::frame_root() {

  this->setTitle("SimpleModManager");
  this->setFooterText(GlobalObjects::_version_str_);
  this->setIcon("romfs:/images/icon.jpg");
  this->addTab("Game Browser", new tab_games());
  this->addSeparator();
  this->addTab("Settings", new tab_general_settings());
  this->addTab("About", new tab_about());

}

bool frame_root::onCancel() {

  auto* lastFocus = brls::Application::getCurrentFocus();

  bool onCancel = TabFrame::onCancel();

  if(lastFocus == brls::Application::getCurrentFocus()){
    brls::Application::quit();
  }

  return onCancel;
}
