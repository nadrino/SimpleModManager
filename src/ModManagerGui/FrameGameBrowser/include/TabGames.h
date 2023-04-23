//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABGAMES_H
#define SIMPLEMODMANAGER_TABGAMES_H

#include <borealis.hpp>

#include "vector"


struct GameItem;
class FrameRoot;

class TabGames : public brls::List {

public:
  TabGames(FrameRoot* frameRoot_);

private:
  FrameRoot* _frameRoot_{};
  std::vector<GameItem> _gameList_;

};

struct GameItem{
  std::string title{};
  int nMods{-1};

  // memory is fully handled by brls
  brls::ListItem* item{nullptr};
};


#endif //SIMPLEMODMANAGER_TABGAMES_H
