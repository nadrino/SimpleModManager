//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABGAMES_H
#define SIMPLEMODMANAGER_TABGAMES_H


#include "ConfigHandler.h"
#include "Selector.h"
#include "GameBrowser.h"

#include <borealis.hpp>

#include "vector"


struct GameItem;
class FrameRoot;

class TabGames : public brls::List {

public:
  explicit TabGames(FrameRoot* owner_);

  // non native getters
  [[nodiscard]] const GameBrowser& getGameBrowser() const;
  [[nodiscard]] const Selector& getGameSelector() const;
  [[nodiscard]] const ConfigHolder& getConfig() const;

  Selector& getGameSelector();
  GameBrowser& getGameBrowser();
  ConfigHolder& getConfig();

private:
  FrameRoot* _owner_{};
  std::vector<GameItem> _gameList_;

};

struct GameItem{
  std::string title{};
  int nMods{-1};

  // memory is fully handled by brls
  brls::ListItem* item{nullptr};
};


#endif //SIMPLEMODMANAGER_TABGAMES_H
