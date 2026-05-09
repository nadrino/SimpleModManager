//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "FrameRoot.h"

#include <TabGames.h>
#include <TabImportMod.h>
#include <TabAbout.h>
#include <TabGeneralSettings.h>

#include "SystemStatusOverlay.h"

#include "Toolbox.h"

#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[FrameRoot]");
});


FrameRoot::FrameRoot() {
  LogWarning << "Build root frame..." << std::endl;

  this->setTitle("SimpleModManager");
  this->setFooterText( "v" + Toolbox::getAppVersion() );
  this->setIcon("romfs:/images/icon_corner.png");
  this->addTab( "Game Browser", new TabGames(this) );
  this->addTab( "Import Mod", new TabImportMod() );
  this->addSeparator();
  this->addTab( "Settings", new TabGeneralSettings(this) );
  this->addTab( "About", new TabAbout() );

  LogInfo << "Root frame built." << std::endl;
}

void FrameRoot::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) {
  brls::TabFrame::draw(vg, x, y, width, height, style, ctx);
  SystemStatusOverlay::draw(vg, x, y, width, style, ctx);
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
