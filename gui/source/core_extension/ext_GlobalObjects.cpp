//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_GlobalObjects.h"
#include <FrameModBrowser.h>
#include <TabModPresets.h>

namespace GuiGlobalObjects {

  static TabModBrowser* _currentTabModBrowserPtr_;
  static FrameModBrowser* _currentFrameModBrowserPtr_;
  static TabModPresets* _currentTabModPresetPtr_;

  void setCurrentTabModBrowserPtr(TabModBrowser* currentTabModBrowserPtr_){
    GuiGlobalObjects::_currentTabModBrowserPtr_ = currentTabModBrowserPtr_;
  }
  void setCurrentFrameModBrowserPtr(FrameModBrowser* currentFrameModBrowserPtr_){
    GuiGlobalObjects::_currentFrameModBrowserPtr_ = currentFrameModBrowserPtr_;
  }
  void setCurrentTabModPresetPtr(TabModPresets* currentTabModPresetPtr_){
    GuiGlobalObjects::_currentTabModPresetPtr_ = currentTabModPresetPtr_;
  }

  TabModBrowser* getCurrentTabModBrowserPtr(){
    return GuiGlobalObjects::_currentTabModBrowserPtr_;
  }
  FrameModBrowser* getCurrentFrameModBrowserPtr(){
    return GuiGlobalObjects::_currentFrameModBrowserPtr_;
  }
  TabModPresets* getCurrentTabModPresetPtr(){
    return GuiGlobalObjects::_currentTabModPresetPtr_;
  }

};