//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODBROWSER_H
#define SIMPLEMODMANAGER_TABMODBROWSER_H

#include <borealis.hpp>
#include <GuiModManager.h>

class TabModBrowser : public brls::List {

public:
  TabModBrowser();

  GuiModManager &getExtModManager();
  std::map<std::string, brls::ListItem *> &getModsListItems();

  void setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_);
  void updateDisplayedModsStatus();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;


private:
  brls::Dialog* dialog;
  std::map<std::string, brls::ListItem*> _modsListItems_;
  GuiModManager _extModManager_;
  bool triggerUpdateModsDisplayedStatus;
  bool triggerRecheckAllMods;
  int frameCounter;


};


#endif //SIMPLEMODMANAGER_TABMODBROWSER_H
