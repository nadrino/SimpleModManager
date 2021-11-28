//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <tab_games.h>
#include <GlobalObjects.h>
#include <frame_mod_browser.h>
#include "toolbox.h"

tab_games::tab_games() {

  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {

    std::string selected_folder = mod_folders_list[i_folder];
    int nb_mods = toolbox::get_list_of_subfolders_in_folder( GlobalObjects::get_mod_browser().get_current_directory() + "/" + selected_folder).size();
    auto* item = new brls::ListItem(selected_folder, "", std::to_string(nb_mods) + " mod(s) available.");
    item->setValue(GlobalObjects::get_mod_browser().get_selector().get_tag(i_folder));
    auto* icon = GlobalObjects::get_mod_browser().get_folder_icon(selected_folder);
    if(icon != nullptr){
      item->setThumbnail(icon, 0x20000);
    }
    item->getClickEvent()->subscribe([this, selected_folder](View* view) {
      brls::Logger::debug("Openning %s", selected_folder.c_str());
      auto* mods_browser = new frame_mod_browser(selected_folder);
      brls::Application::pushView(mods_browser, brls::ViewAnimation::SLIDE_LEFT);
      mods_browser->registerAction("", brls::Key::PLUS, []{return true;}, true);
      mods_browser->updateActionHint(brls::Key::PLUS, ""); // make the change visible
    });
    item->updateActionHint(brls::Key::A, "Open");

    _games_list_.emplace_back(item);
    _nb_mods_list_.emplace_back(nb_mods);
  }

  std::function<bool(int&, int&)> lambda = [](int & a, int & b){ return a > b; };
  auto p = toolbox::sort_permutation(_nb_mods_list_, lambda);
  _games_list_ = toolbox::apply_permutation(_games_list_, p);
  _nb_mods_list_ = toolbox::apply_permutation(_nb_mods_list_, p);

  for(const auto& game_list_item : _games_list_){
    this->addView(game_list_item);
  }

  if(_games_list_.empty()){

    auto* emptyListLabel = new brls::ListItem(
      "No game folders have been found in " + GlobalObjects::get_mod_browser().get_base_folder(),
      "- To add mods, you need to copy them such as: " + GlobalObjects::get_mod_browser().get_base_folder()
      + "<name-of-the-game>/<name-of-the-mod>/<mods-files-and-folders>.\n" \
      "- You can also change this folder (" + GlobalObjects::get_mod_browser().get_base_folder()
      + ") by editing the config file in /config/SimpleModManager/parameters.ini"
      );
    emptyListLabel->show([](){}, false);
    this->addView(emptyListLabel);

  }

}
