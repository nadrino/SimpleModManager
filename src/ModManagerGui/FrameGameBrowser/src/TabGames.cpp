//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <TabGames.h>
#include <GlobalObjects.h>
#include <FrameModBrowser.h>
#include "FrameRoot.h"

#include "Logger.h"
#include "GenericToolbox.Switch.h"

#include "sstream"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabGames]");
});

TabGames::TabGames(FrameRoot* owner_) : _owner_(owner_) {
  LogWarning << "Building game tab..." << std::endl;

  auto gameList = this->getGameBrowser().getSelector().getEntryList();

  if( gameList.empty() ){
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
  }
  else{
    LogInfo << "Adding " << gameList.size() << " game folders..." << std::endl;

    _gameList_.reserve( gameList.size() );
    for( auto& gameEntry : gameList ){
      LogScopeIndent;
      LogInfo << "Adding game folder: \"" << gameEntry.title << "\"" << std::endl;

      std::string gamePath{GenericToolbox::joinPath(this->getConfig().baseFolder, gameEntry.title)};
      int nMods = int( GenericToolbox::getListOfSubFoldersInFolder(gamePath).size() );

      // memory allocation
      auto* item = new brls::ListItem(gameEntry.title, "", std::to_string(nMods) + " mod(s) available.");

      // looking for tid is quite slow... Slows down the boot up
      std::string _titleId_{ GenericToolbox::Switch::Utils::lookForTidInSubFolders(gamePath) };
      auto* icon = GenericToolbox::Switch::Utils::getIconFromTitleId(_titleId_);
      if(icon != nullptr){ item->setThumbnail(icon, 0x20000); }
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
      _gameList_.back().nMods = nMods;

    }
  }

  LogDebug << "Sorting games wrt nb of mods..." << std::endl;
  GenericToolbox::sortVector(_gameList_, [](const GameItem& a_, const GameItem& b_){
    if( a_.nMods > b_.nMods ) return true;
    return false;
  });
  LogDebug << "Sort done." << std::endl;

  // add to the view
  for( auto& game : _gameList_ ){ this->addView( game.item ); }

  LogInfo << "Game tab build." << std::endl;
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
