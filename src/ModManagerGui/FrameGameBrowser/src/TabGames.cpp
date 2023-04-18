//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <TabGames.h>
#include <GlobalObjects.h>
#include <FrameModBrowser.h>

#include "Logger.h"
#include "GenericToolbox.Switch.h"

#include "sstream"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabGames]");
});

TabGames::TabGames() {
  LogWarning << "Building game tab..." << std::endl;

  auto gameFolderList = GlobalObjects::getModBrowser().getSelector().getSelectionList();

  if( gameFolderList.empty() ){
    LogInfo << "No game found." << std::endl;

    std::stringstream ssTitle;
    std::stringstream ssSubTitle;

    ssTitle << "No game folders have been found in " << GlobalObjects::getModBrowser().get_base_folder();

    ssSubTitle << "- To add mods, you need to copy them such as: " << GlobalObjects::getModBrowser().get_base_folder();
    ssSubTitle << "<name-of-the-game>/<name-of-the-mod>/<mods-files-and-folders>." << std::endl;
    ssSubTitle << "- You can also change this folder (" + GlobalObjects::getModBrowser().get_base_folder();
    ssSubTitle << ") by editing the config file in /config/SimpleModManager/parameters.ini";

    _gameList_.emplace_back();
    _gameList_.back().item = new brls::ListItem( ssTitle.str(), ssSubTitle.str() );
    _gameList_.back().item->show([](){}, false);
  }
  else{
    LogInfo << "Adding " << gameFolderList.size() << " game folders..." << std::endl;

    _gameList_.reserve( gameFolderList.size() );
    int iGame{-1};
    for( auto& gameFolder : gameFolderList ){
      LogScopeIndent;
      LogInfo << "Adding game folder: \"" << gameFolder << "\"" << std::endl;

      iGame++;

      int nMods = int( GenericToolbox::getListOfSubFoldersInFolder(
          GlobalObjects::getModBrowser().get_current_directory() + "/" + gameFolder
      ).size() );

      // memory allocation
      auto* item = new brls::ListItem(gameFolder, "", std::to_string(nMods) + " mod(s) available.");
      item->setValue(GlobalObjects::getModBrowser().getSelector().getTag(iGame));

      // TODO: should be moved to a utils library
      auto* icon = GlobalObjects::getModBrowser().getFolderIcon( gameFolder );
      if(icon != nullptr){ item->setThumbnail(icon, 0x20000); }
      item->getClickEvent()->subscribe([gameFolder](View* view) {
        LogWarning << "Opening \"" << gameFolder << "\"" << std::endl;
        auto* mods_browser = new FrameModBrowser(gameFolder);
        brls::Application::pushView(mods_browser, brls::ViewAnimation::SLIDE_LEFT);
        mods_browser->registerAction("", brls::Key::PLUS, []{return true;}, true);
        mods_browser->updateActionHint(brls::Key::PLUS, ""); // make the change visible
      });
      item->updateActionHint(brls::Key::A, "Open");


      _gameList_.emplace_back();
      _gameList_.back().title = gameFolder;
      _gameList_.back().item = item;
      _gameList_.back().nMods = nMods;

    }
  }

  LogDebug << "Sorting games wrt nb of mods..." << std::endl;
  std::function<bool(const GameItem& a_, const GameItem& b_)> aGoesFirst([](const GameItem& a_, const GameItem& b_){
    if( a_.nMods > b_.nMods ) return true;
    return false;
  });
  GenericToolbox::sortVector(_gameList_, aGoesFirst);
  LogDebug << "Sort done." << std::endl;

  // add to the view
  for( auto& game : _gameList_ ){ this->addView( game.item ); }

  LogInfo << "Game tab build." << std::endl;
}
