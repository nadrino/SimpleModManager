//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "tab_mod_browser.h"
#include <GlobalObjects.h>

tab_mod_browser::tab_mod_browser() {



  // Setup the list
  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
    std::string selected_mod = mod_folders_list[i_folder];
    auto* item = new brls::ListItem(selected_mod, "", "");
    item->getClickEvent()->subscribe([this, item, selected_mod](View* view) {
      brls::Logger::debug("Applying mod: %s", selected_mod.c_str());
      // apply mode
      std::string dialogTitle = "Applying: " + selected_mod + "\n";

      GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(selected_mod, true);

//      auto* frame = new brls::AppletFrame(false, false);
//      auto* progressBar = new brls::ProgressDisplay();
//      frame->setContentView(progressBar);
//      brls::PopupFrame::open("Popup title", BOREALIS_ASSET("icon/borealis.jpg"), frame, "Subtitle left", "Subtitle right");

      item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod));
    });
    item->registerAction("Disable", brls::Key::X, [item, selected_mod]{
      GlobalObjects::get_mod_browser().get_mod_manager().remove_mod(selected_mod);
      item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod));
      return true;
    });
    item->updateActionHint(brls::Key::A, "Apply");

    this->addView(item);
    _mods_list_.emplace_back(item);
  }

  tab_mod_browser::updateModsStatus();

}

void tab_mod_browser::updateModsStatus() {

  for(auto& modItem : _mods_list_){
    modItem->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(
      modItem->getLabel()
      ));
  }

}
