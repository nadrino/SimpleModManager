//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABGAMES_H
#define SIMPLEMODMANAGER_TABGAMES_H


#include "ConfigHandler.h"
#include "GameBrowser.h"
#include "Selector.h"

#include "borealis.hpp"

#include <vector>


struct GameItem;
class FrameRoot;

class TabGames : public brls::List {

public:
  explicit TabGames(FrameRoot* owner_);

  void rebuildLayout(bool force_ = false);
  void willAppear(bool resetState = false) override;

  // non native getters
  [[nodiscard]] const GameBrowser& getGameBrowser() const;
  [[nodiscard]] const ConfigHolder& getConfig() const;

  GameBrowser& getGameBrowser();
  ConfigHolder& getConfig();

private:
  FrameRoot* _owner_{};
  std::vector<GameItem> _gameList_;
  bool _hasAppearedOnce_{false};
  bool _layoutBuilt_{false};

};

struct GameItem{
  std::string title{};
  int nMods{-1};

  // memory is fully handled by brls
  brls::ListItem* item{nullptr};
};


#endif //SIMPLEMODMANAGER_TABGAMES_H
