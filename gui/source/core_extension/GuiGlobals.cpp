//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "GuiGlobals.h"
#include <FrameModBrowser.h>
#include <TabModPresets.h>

namespace GuiGlobals {

  static TabModBrowser* currentTabModBrowserPtr;
  static FrameModBrowser* currentFrameModBrowserPtr;
  static TabModPresets* _currentTabModPresetPtr_;

  void setCurrentTabModBrowserPtr(TabModBrowser* currentTabModBrowserPtr_){
    GuiGlobals::currentTabModBrowserPtr = currentTabModBrowserPtr_;
  }
  void setCurrentFrameModBrowserPtr(FrameModBrowser* currentFrameModBrowserPtr_){
    GuiGlobals::currentFrameModBrowserPtr = currentFrameModBrowserPtr_;
  }
  void setCurrentTabModPresetPtr(TabModPresets* currentTabModPresetPtr_){
    GuiGlobals::_currentTabModPresetPtr_ = currentTabModPresetPtr_;
  }

  TabModBrowser* getCurrentTabModBrowserPtr(){
    return GuiGlobals::currentTabModBrowserPtr;
  }
  FrameModBrowser* getCurrentFrameModBrowserPtr(){
    return GuiGlobals::currentFrameModBrowserPtr;
  }
  TabModPresets* getCurrentTabModPresetPtr(){
    return GuiGlobals::_currentTabModPresetPtr_;
  }

};