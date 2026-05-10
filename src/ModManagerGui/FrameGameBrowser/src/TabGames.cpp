//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "TabGames.h"
#include "FrameModBrowser.h"
#include "FrameRoot.h"
#include "Toolbox.h"

#include "GenericToolbox.Switch.h"
#include "GenericToolbox.Vector.h"
#include "Logger.h"

#include <cstdlib>
#include <sstream>

LoggerInit([]{
  Logger::setUserHeaderStr("[TabGames]");
});

TabGames::TabGames(FrameRoot* owner_) : _owner_(owner_) {
  LogWarning << "Building game tab..." << std::endl;
  this->rebuildLayout(false);
  LogInfo << "Game tab build." << std::endl;
}

void TabGames::willAppear(bool resetState) {
  brls::List::willAppear(resetState);

  if( _hasAppearedOnce_ ){
    if( _restoreFocusAfterModBrowser_ ){
      _refreshGameStatusOnNextDraw_ = true;
      _restoreFocusOnNextDraw_ = true;
    }
    else{
      _refreshOnNextDraw_ = true;
    }
    _restoreFocusAfterModBrowser_ = false;
    this->invalidate();
  }
  _hasAppearedOnce_ = true;
}

void TabGames::rebuildLayout(bool force_) {
  if( _layoutBuilt_ and not getGameBrowser().refreshGameList(force_) ){
    this->resyncListItemFocusIndices();
    return;
  }

  this->clear(true);
  _gameList_.clear();

  auto gameList = this->getGameBrowser().getSelector().getEntryList();

  auto addNoGamesItem = [this]() {
    LogInfo << "No game found." << std::endl;

    std::stringstream ssTitle;
    std::stringstream ssSubTitle;

    auto baseFolder = this->getConfig().baseFolder;
    ssTitle << "No game folders have been found in " << baseFolder;

    ssSubTitle << "- To add mods, you need to copy them such as: ";
    ssSubTitle << baseFolder << "/<name-of-the-game>/<name-of-the-mod>/<mods-files-and-folders>." << std::endl;
    ssSubTitle << "- You can also change this folder (" + baseFolder;
    ssSubTitle << ") by editing the config file in /config/SimpleModManager/parameters.ini";

    _gameList_.emplace_back();
    _gameList_.back().item = new brls::ListItem( ssTitle.str(), ssSubTitle.str() );
    _gameList_.back().item->show([](){}, false);
  };

  if( gameList.empty() ){
    addNoGamesItem();
  }
  else{
    LogInfo << "Adding " << gameList.size() << " game folders..." << std::endl;

    _gameList_.reserve( gameList.size() );
    for( auto& gameEntry : gameList ){
      LogScopeIndent;
      LogDebug << "Adding game folder: \"" << gameEntry.title << "\"" << std::endl;

      std::string gamePath{GenericToolbox::joinPath(this->getConfig().baseFolder, gameEntry.title)};

      auto* icon = Toolbox::getGameFolderIcon(gamePath);
      if( icon == nullptr ){
        LogInfo << "Skipping game folder without icon: \"" << gameEntry.title << "\"" << std::endl;
        continue;
      }

      // memory allocation
      const std::string tag = gameEntry.tag.empty() ? "" : gameEntry.tag;
      auto* item = new brls::ListItem(gameEntry.title, "", tag);
      item->setThumbnail(icon, 0x20000);
      delete[] icon;

      item->getClickEvent()->subscribe([&, gameEntry](View* view) {
        LogWarning << "Opening \"" << gameEntry.title << "\"" << std::endl;
        _focusGameNameAfterReturn_ = gameEntry.title;
        _restoreFocusAfterModBrowser_ = true;
        getGameBrowser().selectGame( gameEntry.title );
        auto* modsBrowser = new FrameModBrowser( &_owner_->getGuiModManager() );
        brls::Application::pushView(modsBrowser, brls::ViewAnimation::SLIDE_LEFT);
        modsBrowser->registerAction("", brls::Key::PLUS, []{return true;}, true);
        modsBrowser->updateActionHint(brls::Key::PLUS, ""); // make the change visible
      });
      item->updateActionHint(brls::Key::A, "Open");


      _gameList_.emplace_back();
      _gameList_.back().title = gameEntry.title;
      _gameList_.back().item = item;

    }

    if( _gameList_.empty() ){
      addNoGamesItem();
    }
  }

  // add to the view
  for( auto& game : _gameList_ ){ this->addView( game.item ); }
  this->resyncListItemFocusIndices();
  _layoutBuilt_ = true;

}

void TabGames::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) {
  const bool viewTransitionRunning = brls::Application::hasViewDisappearing();

  if( _refreshOnNextDraw_ ){
    if( not viewTransitionRunning ){
      _refreshOnNextDraw_ = false;
      this->rebuildLayout(false);
    }
  }

  if( _refreshGameStatusOnNextDraw_ ){
    if( not viewTransitionRunning ){
      _refreshGameStatusOnNextDraw_ = false;
      this->refreshDisplayedGameStatus(_focusGameNameAfterReturn_);
    }
  }

  if( _restoreFocusOnNextDraw_ ){
    if( not viewTransitionRunning ){
      _restoreFocusOnNextDraw_ = false;
      this->restoreFocusAfterRebuild();
    }
  }

  brls::ScrollView::draw(vg, x, y, width, height, style, ctx);
}

brls::View* TabGames::getDefaultFocus() {
  if( _gameList_.empty() ){
    return nullptr;
  }
  return _gameList_.front().item;
}

void TabGames::resyncListItemFocusIndices() {
  // Borealis navigation uses parentUserData as the child index. After a full
  // clear/rebuild the old focused item can be gone, so refresh every child index.
  for( size_t i = 0; i < this->getViewsCount(); ++i ) {
    auto* child = this->getChild( i );
    if( child == nullptr ) {
      continue;
    }
    auto* parent = child->getParent();
    if( parent == nullptr ) {
      continue;
    }

    auto* userdata = static_cast<size_t*>( malloc( sizeof(size_t) ) );
    *userdata = i;
    child->setParent( parent, userdata );
  }
}

brls::ListItem* TabGames::findGameItem(const std::string& gameTitle_) const {
  if( gameTitle_.empty() ){
    return nullptr;
  }

  for( const auto& game : _gameList_ ){
    if( game.item != nullptr && game.item->getLabel() == gameTitle_ ){
      return game.item;
    }
  }
  return nullptr;
}

void TabGames::refreshDisplayedGameStatus(const std::string& gameTitle_) {
  auto* item = this->findGameItem(gameTitle_);
  if( item == nullptr ){
    return;
  }

  const std::string tag = this->getGameBrowser().refreshGameListTag(gameTitle_);
  item->setSubLabel(tag);
  item->invalidate(true);
  this->invalidate(true);
  if( this->getParent() != nullptr ){
    this->getParent()->invalidate(true);
  }
}

void TabGames::restoreFocusAfterRebuild() {
  if( _gameList_.empty() ){
    return;
  }
  if( this->getParent() == nullptr and not _hasAppearedOnce_ ){
    return;
  }

  auto* focusItem = this->findGameItem(_focusGameNameAfterReturn_);
  if( focusItem == nullptr ){
    focusItem = _gameList_.front().item;
  }

  if( focusItem != nullptr ){
    brls::Application::giveFocus(focusItem);
    this->invalidate(true);
    if( this->getParent() != nullptr ){
      this->getParent()->invalidate(true);
    }
  }
}

const GameBrowser& TabGames::getGameBrowser() const{
  return _owner_->getGuiModManager().getGameBrowser();
}
const ConfigHolder& TabGames::getConfig() const{
  return getGameBrowser().getConfigHandler().getConfig();
}

GameBrowser& TabGames::getGameBrowser(){
  return _owner_->getGuiModManager().getGameBrowser();
}
ConfigHolder& TabGames::getConfig(){
  return getGameBrowser().getConfigHandler().getConfig();
}
