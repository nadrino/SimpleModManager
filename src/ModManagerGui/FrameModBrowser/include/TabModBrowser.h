//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODBROWSER_H
#define SIMPLEMODMANAGER_TABMODBROWSER_H


#include <GuiModManager.h>
#include "ModManager.h"

#include <borealis.hpp>

#include "map"
#include "string"


struct ModItem;
class FrameModBrowser;

class TabModBrowser : public brls::List {

public:
  explicit TabModBrowser(FrameModBrowser* owner_);

  void updateDisplayedModsStatus();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

  [[nodiscard]] const ModManager& getModManager() const;

private:
  FrameModBrowser* _owner_{nullptr};
  std::vector<ModItem> _modItemList_{};

  bool triggerRecheckAllMods{false};

};


struct ModItem{
  int modIndex{-1};

  // memory is handled by brls -> could be lost in the wild but handy to keep somewhere
  brls::ListItem* item{nullptr}; // deleted in BoxLayout::~BoxLayout()
};


#endif //SIMPLEMODMANAGER_TABMODBROWSER_H
