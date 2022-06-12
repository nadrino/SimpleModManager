//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABMODOPTIONS_H
#define SIMPLEMODMANAGER_TABMODOPTIONS_H

#include <borealis.hpp>
#include <TabModBrowser.h>

class TabModOptions : public brls::List {

public:
  TabModOptions();

  void initialize();

  void buildFolderInstallPresetItem();
  void buildResetModsCacheItem();
  void buildDisableAllMods();
  void buildGameIdentificationItem();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

  // FolderInstallPreset
  int _preSelection_;
  std::string _inheritedTitle_;

  // menu items
  brls::ListItem* _itemFolderInstallPreset_;
  brls::ListItem* _itemResetModsCache_;
  brls::ListItem* _itemDisableAllMods_;
  brls::ListItem* _itemGameIdentification_;

  bool doUpdateModsStatus;
  int frameSkipCount;

};


#endif //SIMPLEMODMANAGER_TABMODOPTIONS_H
