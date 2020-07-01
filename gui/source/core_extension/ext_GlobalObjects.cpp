//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_GlobalObjects.h"
#include <frame_mod_browser.h>
#include <tab_mod_presets.h>

namespace ext_GlobalObjects {

  static tab_mod_browser* _currentTabModBrowserPtr_;
  static frame_mod_browser* _currentFrameModBrowserPtr_;
  static tab_mod_presets* _currentTabModPresetPtr_;

  void setCurrentTabModBrowserPtr(tab_mod_browser* currentTabModBrowserPtr_){
    ext_GlobalObjects::_currentTabModBrowserPtr_ = currentTabModBrowserPtr_;
  }
  void setCurrentFrameModBrowserPtr(frame_mod_browser* currentFrameModBrowserPtr_){
    ext_GlobalObjects::_currentFrameModBrowserPtr_ = currentFrameModBrowserPtr_;
  }
  void setCurrentTabModPresetPtr(tab_mod_presets* currentTabModPresetPtr_){
    ext_GlobalObjects::_currentTabModPresetPtr_ = currentTabModPresetPtr_;
  }

  tab_mod_browser* getCurrentTabModBrowserPtr(){
    return ext_GlobalObjects::_currentTabModBrowserPtr_;
  }
  frame_mod_browser* getCurrentFrameModBrowserPtr(){
    return ext_GlobalObjects::_currentFrameModBrowserPtr_;
  }
  tab_mod_presets* getCurrentTabModPresetPtr(){
    return ext_GlobalObjects::_currentTabModPresetPtr_;
  }

};