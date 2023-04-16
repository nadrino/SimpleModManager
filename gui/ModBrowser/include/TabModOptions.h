//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODOPTIONS_H
#define SIMPLEMODMANAGER_TABMODOPTIONS_H

#include <TabModBrowser.h>

#include <borealis.hpp>


class FrameModBrowser;

class TabModOptions : public brls::List {

public:
  explicit TabModOptions(FrameModBrowser* owner_);

  void initialize();

  void buildFolderInstallPresetItem();
  void buildResetModsCacheItem();
  void buildDisableAllMods();
  void buildGameIdentificationItem();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

private:
  FrameModBrowser* _owner_{nullptr};

  // FolderInstallPreset
  bool doUpdateModsStatus{false};
  int frameSkipCount{0};
  int _preSelection_{0};
  std::string _inheritedTitle_{"Inherited from the main menu"};

  // memory handled by brls
  brls::ListItem* _itemFolderInstallPreset_{nullptr};
  brls::ListItem* _itemResetModsCache_{nullptr};
  brls::ListItem* _itemDisableAllMods_{nullptr};
  brls::ListItem* _itemGameIdentification_{nullptr};

};


#endif //SIMPLEMODMANAGER_TABMODOPTIONS_H
