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
  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
  brls::View* getDefaultFocus() override;

  // non native getters
  [[nodiscard]] const GameBrowser& getGameBrowser() const;
  [[nodiscard]] const ConfigHolder& getConfig() const;

  GameBrowser& getGameBrowser();
  ConfigHolder& getConfig();

private:
  void resyncListItemFocusIndices();
  brls::ListItem* findGameItem(const std::string& gameTitle_) const;
  void refreshDisplayedGameStatus(const std::string& gameTitle_);
  void restoreFocusAfterRebuild();

  FrameRoot* _owner_{};
  std::vector<GameItem> _gameList_;
  std::string _focusGameNameAfterReturn_{};
  bool _restoreFocusAfterModBrowser_{false};
  bool _refreshOnNextDraw_{false};
  bool _refreshGameStatusOnNextDraw_{false};
  bool _restoreFocusOnNextDraw_{false};
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
