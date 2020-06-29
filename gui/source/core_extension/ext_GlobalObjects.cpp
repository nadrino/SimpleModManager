//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_GlobalObjects.h"

namespace ext_GlobalObjects {

  static tab_mod_browser* _currentTabModBrowserPtr_;

  void setCurrentTabModBrowserPtr(tab_mod_browser* currentTabModBrowserPtr_){
    ext_GlobalObjects::_currentTabModBrowserPtr_ = currentTabModBrowserPtr_;
  }

  tab_mod_browser* getCurrentTabModBrowserPtr(){
    return ext_GlobalObjects::_currentTabModBrowserPtr_;
  }

};