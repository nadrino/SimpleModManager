//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H

#include <tab_mod_browser.h>
#include <frame_mod_browser.h>

namespace ext_GlobalObjects {


  void setCurrentTabModBrowserPtr(tab_mod_browser* currentTabModBrowserPtr_);
  void setCurrentFrameModBrowserPtr(frame_mod_browser* currentFrameModBrowserPtr_);
  void setCurrentTabModPresetPtr(tab_mod_presets* currentTabModPresetPtr_);

  tab_mod_browser* getCurrentTabModBrowserPtr();
  frame_mod_browser* getCurrentFrameModBrowserPtr();
  tab_mod_presets* getCurrentTabModPresetPtr();

};


#endif //SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
