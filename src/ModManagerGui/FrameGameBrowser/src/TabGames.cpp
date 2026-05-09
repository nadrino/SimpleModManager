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
    this->rebuildLayout(false);
  }
  _hasAppearedOnce_ = true;
}

void TabGames::rebuildLayout(bool force_) {
  if( _layoutBuilt_ and not getGameBrowser().refreshGameList(force_) ){
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
  _layoutBuilt_ = true;

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
