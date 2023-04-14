//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODBROWSER_H
#define SIMPLEMODMANAGER_TABMODBROWSER_H

#include <borealis.hpp>
#include <GuiModManager.h>

#include "map"
#include "string"


struct ModItem;


class TabModBrowser : public brls::List {

public:
  TabModBrowser();

  GuiModManager &getExtModManager();

  void setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_);
  void updateDisplayedModsStatus();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;


private:
  bool triggerUpdateModsDisplayedStatus{false};
  bool triggerRecheckAllMods{false};

  GuiModManager _extModManager_{};
  std::vector<ModItem> _modList_{};


};


struct ModItem{
  std::string title{};

  // memory is handled by brls -> could be lost in the wild but handy to keep somewhere
  brls::ListItem* item{nullptr}; // deleted in BoxLayout::~BoxLayout()
  brls::Dialog* disableDialog{nullptr};
};


#endif //SIMPLEMODMANAGER_TABMODBROWSER_H
