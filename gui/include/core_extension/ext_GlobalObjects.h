//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H

#include <tab_mod_browser.h>

namespace ext_GlobalObjects {

  static tab_mod_browser* _currentTabModBrowserPtr_;

  void setCurrentTabModBrowserPtr(tab_mod_browser* currentTabModBrowserPtr_);

  tab_mod_browser* getCurrentTabModBrowserPtr();

};


#endif //SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
