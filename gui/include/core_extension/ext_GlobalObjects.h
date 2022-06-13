//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H

#include <TabModBrowser.h>
#include <FrameModBrowser.h>

namespace GuiGlobalObjects {


  void setCurrentTabModBrowserPtr(TabModBrowser* currentTabModBrowserPtr_);
  void setCurrentFrameModBrowserPtr(FrameModBrowser* currentFrameModBrowserPtr_);
  void setCurrentTabModPresetPtr(TabModPresets* currentTabModPresetPtr_);

  TabModBrowser* getCurrentTabModBrowserPtr();
  FrameModBrowser* getCurrentFrameModBrowserPtr();
  TabModPresets* getCurrentTabModPresetPtr();

};


#endif //SIMPLEMODMANAGER_EXT_GLOBALOBJECTS_H
