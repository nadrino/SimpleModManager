//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_GlobalObjects.h"
#include <FrameModBrowser.h>
#include <TabModPresets.h>

namespace ext_GlobalObjects {

  static TabModBrowser* _currentTabModBrowserPtr_;
  static FrameModBrowser* _currentFrameModBrowserPtr_;
  static TabModPresets* _currentTabModPresetPtr_;

  void setCurrentTabModBrowserPtr(TabModBrowser* currentTabModBrowserPtr_){
    ext_GlobalObjects::_currentTabModBrowserPtr_ = currentTabModBrowserPtr_;
  }
  void setCurrentFrameModBrowserPtr(FrameModBrowser* currentFrameModBrowserPtr_){
    ext_GlobalObjects::_currentFrameModBrowserPtr_ = currentFrameModBrowserPtr_;
  }
  void setCurrentTabModPresetPtr(TabModPresets* currentTabModPresetPtr_){
    ext_GlobalObjects::_currentTabModPresetPtr_ = currentTabModPresetPtr_;
  }

  TabModBrowser* getCurrentTabModBrowserPtr(){
    return ext_GlobalObjects::_currentTabModBrowserPtr_;
  }
  FrameModBrowser* getCurrentFrameModBrowserPtr(){
    return ext_GlobalObjects::_currentFrameModBrowserPtr_;
  }
  TabModPresets* getCurrentTabModPresetPtr(){
    return ext_GlobalObjects::_currentTabModPresetPtr_;
  }

};