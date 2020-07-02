//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_MOD_PRESETS_H
#define SIMPLEMODMANAGER_TAB_MOD_PRESETS_H

#include <borealis.hpp>

class tab_mod_presets : public brls::List {

public:
  tab_mod_presets();

  void assignButtons(brls::ListItem *item, bool isPreset_);
  void updatePresetItems();

  int getNbFreeSlots() const;

private:
  int _maxNbPresetsSlots_;
  int _nbFreeSlots_;
  std::vector<brls::ListItem*> _itemList_;
  brls::ListItem* _itemNewCreatePreset_;

};


#endif //SIMPLEMODMANAGER_TAB_MOD_PRESETS_H
