//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_MOD_PRESETS_H
#define SIMPLEMODMANAGER_TAB_MOD_PRESETS_H

#include <borealis.hpp>

class tab_mod_presets : public brls::List {

public:
  tab_mod_presets();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

  void setTriggerUpdate(bool triggerUpdate);

  std::vector<brls::ListItem *> &getItemList();

  brls::ListItem *createNewPresetItem(const std::string& presetName_);
  void assignButtons(brls::ListItem *item, bool isPreset_);
  void updatePresetItems();

private:
  bool _triggerUpdate_;
  std::vector<brls::ListItem*> _itemList_;
  brls::ListItem* _itemNewCreatePreset_;

};


#endif //SIMPLEMODMANAGER_TAB_MOD_PRESETS_H
