//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_MOD_BROWSER_H
#define SIMPLEMODMANAGER_TAB_MOD_BROWSER_H

#include <borealis.hpp>
#include <ext_mod_manager.h>

class tab_mod_browser : public brls::List {

public:
  tab_mod_browser();

  ext_mod_manager &getExtModManager();
  std::map<std::string, brls::ListItem *> &getModsListItems();

  void setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_);
  void updateDisplayedModsStatus();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;


private:
  brls::Dialog* dialog;
  std::map<std::string, brls::ListItem*> _modsListItems_;
  ext_mod_manager _extModManager_;
  bool triggerUpdateModsDisplayedStatus;
  bool triggerRecheckAllMods;
  int frameCounter;


};


#endif //SIMPLEMODMANAGER_TAB_MOD_BROWSER_H
