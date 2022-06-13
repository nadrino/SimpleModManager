//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <TabGames.h>
#include <GlobalObjects.h>
#include <FrameModBrowser.h>

#include "Logger.h"
#include "GenericToolbox.Switch.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabGames]");
});

TabGames::TabGames() {
  LogInfo << __METHOD_NAME__ << std::endl;

  auto modsList = GlobalObjects::getModBrowser().getSelector().getSelectionList();
  for (int i_folder = 0; i_folder < int(modsList.size()); i_folder++) {

    std::string selectedFolder = modsList[i_folder];

    size_t nb_mods = GenericToolbox::getListOfSubFoldersInFolder(
        GlobalObjects::getModBrowser().get_current_directory() + "/" + selectedFolder).size();
    auto* item = new brls::ListItem(selectedFolder, "", std::to_string(nb_mods) + " mod(s) available.");
    item->setValue(GlobalObjects::getModBrowser().getSelector().get_tag(i_folder));
    auto* icon = GlobalObjects::getModBrowser().get_folder_icon(selectedFolder);
    if(icon != nullptr){ item->setThumbnail(icon, 0x20000); }
    item->getClickEvent()->subscribe([selectedFolder](View* view) {
      LogDebug << "Opening \"" << selectedFolder << "\"" << std::endl;
      auto* mods_browser = new FrameModBrowser(selectedFolder);
      brls::Application::pushView(mods_browser, brls::ViewAnimation::SLIDE_LEFT);
      mods_browser->registerAction("", brls::Key::PLUS, []{return true;}, true);
      mods_browser->updateActionHint(brls::Key::PLUS, ""); // make the change visible
    });
    item->updateActionHint(brls::Key::A, "Open");
    item->updateActionHint(brls::Key::B, "Back");

    _gamesList_.emplace_back(item);
    _nbModsPerGameList_.emplace_back(nb_mods);
  }

  std::function<bool(int, int)> lambda = [](int a, int b){ return a > b; };
  auto p = GenericToolbox::getSortPermutation(_nbModsPerGameList_, lambda);
  _gamesList_ = GenericToolbox::applyPermutation(_gamesList_, p);
  _nbModsPerGameList_ = GenericToolbox::applyPermutation(_nbModsPerGameList_, p);

  for(const auto& game_list_item : _gamesList_){
    this->addView(game_list_item);
  }

  if(_gamesList_.empty()){

    auto* emptyListLabel = new brls::ListItem(
      "No game folders have been found in " + GlobalObjects::getModBrowser().get_base_folder(),
      "- To add mods, you need to copy them such as: " + GlobalObjects::getModBrowser().get_base_folder()
      + "<name-of-the-game>/<name-of-the-mod>/<mods-files-and-folders>.\n" \
      "- You can also change this folder (" + GlobalObjects::getModBrowser().get_base_folder()
      + ") by editing the config file in /config/SimpleModManager/parameters.ini"
      );
    emptyListLabel->show([](){}, false);
    this->addView(emptyListLabel);

  }

  LogInfo << __METHOD_NAME__ << std::endl;
}
