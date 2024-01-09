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
  explicit TabModPresets(FrameModBrowser* owner_) : _owner_(owner_) {  }

  void setTriggerUpdateItem(bool triggerUpdateItem){ _triggerUpdateItem_ = triggerUpdateItem; }

  void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) override;

  void assignButtons(brls::ListItem *item, bool isPreset_);


protected:
  void updatePresetItems();

private:
  FrameModBrowser* _owner_{nullptr};

  bool _triggerUpdateItem_{true};

  // memory handled by brls
  brls::ListItem* _itemNewCreatePreset_{nullptr};
  std::vector<brls::ListItem*> _itemList_{};

};


#endif //SIMPLEMODMANAGER_TABMODPRESETS_H
