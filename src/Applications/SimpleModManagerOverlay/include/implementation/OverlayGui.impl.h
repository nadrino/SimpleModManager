//
// Created by Adrien Blanchet on 18/04/2023.
//

#ifndef SIMPLEMODMANAGER_OVERLAYGUI_IMPL_H
#define SIMPLEMODMANAGER_OVERLAYGUI_IMPL_H

#include "OverlayGuiLoader.h"

#include <GameBrowserGui.h>


std::unique_ptr<tsl::Gui> OverlayGuiLoader::loadInitialGui() {
  // Initial Gui to load. It's possible to pass arguments to its constructor like this
  return initially<GameBrowserGui>();
}

void OverlayGuiLoader::initServices() {
  // libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
  tsl::hlp::ScopeGuard dirGuard( [&]{} );
}
void OverlayGuiLoader::exitServices() {
  // Called at the end to clean up all services previously initialized
}

void OverlayGuiLoader::onShow(){
  // Called before overlay wants to change from invisible to visible state
}
void OverlayGuiLoader::onHide(){
  // Called before overlay wants to change from visible to invisible state
}


#endif //SIMPLEMODMANAGER_OVERLAYGUI_IMPL_H
