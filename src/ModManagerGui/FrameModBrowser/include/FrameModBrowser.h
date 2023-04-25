//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAMEMODBROWSER_H
#define SIMPLEMODMANAGER_FRAMEMODBROWSER_H

#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModOptions.h>
#include <TabModPresets.h>

#include "GameBrowser.h"

#include <borealis.hpp>

#include "string"


class FrameModBrowser : public brls::TabFrame {

public:
  explicit FrameModBrowser(GameBrowser* gameBrowser_);
  bool onCancel() override;

  uint8_t *getIcon();
  std::string getTitleId();
  GuiModManager &getModManager();
  TabModBrowser* getTabModBrowser(){ return _tabModBrowser_; }
  TabModPresets* getTabModPresets(){ return _tabModPresets_; }

  const GameBrowser* getGameBrowser() const{ return _gameBrowser_; }
  GameBrowser* getGameBrowser(){ return _gameBrowser_; }


private:
  GameBrowser* _gameBrowser_{};
  GuiModManager _modManager_{};

  // memory handled by brls
  TabModBrowser* _tabModBrowser_{nullptr};
  TabModOptions* _tabModOptions_{nullptr};
  TabModPresets* _tabModPresets_{nullptr};
  TabModPlugins* _tabModPlugins_{nullptr};

  uint8_t* _icon_{nullptr};
  std::string _titleId_{};

};


#endif //SIMPLEMODMANAGER_FRAMEMODBROWSER_H
