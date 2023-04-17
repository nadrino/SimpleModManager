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

  void assignButtons(brls::ListItem *item, bool isPreset_);
  void updatePresetItems();

  [[nodiscard]] int getNbFreeSlots() const;

private:
  FrameModBrowser* _owner_{nullptr};

  int _maxNbPresetsSlots_;
  int _nbFreeSlots_;
  std::vector<brls::ListItem*> _itemList_;
  brls::ListItem* _itemNewCreatePreset_;

};


#endif //SIMPLEMODMANAGER_TABMODPRESETS_H