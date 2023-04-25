//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODPRESETS_H
#define SIMPLEMODMANAGER_TABMODPRESETS_H

#include <borealis.hpp>

#include "vector"

class FrameModBrowser;

class TabModPresets : public brls::List {

public:
  explicit TabModPresets(FrameModBrowser* owner_);

  void setTriggerUpdateItem(bool triggerUpdateItem);

  void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;

  void assignButtons(brls::ListItem *item, bool isPreset_);


protected:
  void updatePresetItems();

private:
  FrameModBrowser* _owner_{nullptr};

  bool _triggerUpdateItem_{true};
  int _nbFreeSlots_{0};
  int _maxNbPresetsSlots_{20};
  std::vector<brls::ListItem*> _itemList_{};
  brls::ListItem* _itemNewCreatePreset_{nullptr};

};


#endif //SIMPLEMODMANAGER_TABMODPRESETS_H
