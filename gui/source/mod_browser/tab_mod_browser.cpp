//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "tab_mod_browser.h"
#include <GlobalObjects.h>
#include <thread>
#include <future>
#include <popup_apply_mod.h>
#include <popup_loading.h>

#include <ext_toolbox.h>
#include <ext_GlobalObjects.h>

tab_mod_browser::tab_mod_browser() {

  isAlreadyChecked = false;

  ext_GlobalObjects::setCurrentTabModBrowserPtr(this);

  // Setup the list
  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
    std::string selected_mod = mod_folders_list[i_folder];
    auto* item = new brls::ListItem(selected_mod, "", "");
    item->getClickEvent()->subscribe([this, selected_mod](View* view) {
      brls::Logger::debug("Applying mod: %s", selected_mod.c_str());
      this->getExtModManager().setModName(selected_mod);
      this->getExtModManager().start_apply_mod();
      return true;
    });
    item->registerAction("Disable", brls::Key::X, [this, selected_mod]{
      brls::Logger::debug("Disabling mod: %s", selected_mod.c_str());
      this->getExtModManager().setModName(selected_mod);
      this->getExtModManager().start_remove_mod();
      return true;
    });
    item->registerAction("Test", brls::Key::Y, [this,selected_mod]{
      this->getExtModManager().setModName(selected_mod);
      this->getExtModManager().start_remove_mod();
      return true;
    });
    item->updateActionHint(brls::Key::A, "Apply");
    item->setValue("Unchecked");

    this->addView(item);
    _modsListItems_[selected_mod] = item;
  }

}

void tab_mod_browser::updateModsStatus() {

  brls::Logger::debug("updateModsStatus");
  this->getExtModManager().start_check_mods_list(
    GlobalObjects::get_mod_browser().get_selector().get_selection_list()
    );

//  for(auto& modItem : _modsListItems_){
//    modItem.second->setValue(
//      GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(
//        modItem.second->getLabel()
//      ));
//  }

}

ext_mod_manager &tab_mod_browser::getExtModManager() {
  return _extModManager_;
}

std::map<std::string, brls::ListItem *> &tab_mod_browser::getModsListItems() {
  return _modsListItems_;
}

void tab_mod_browser::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                           brls::FrameContext *ctx) {
  ScrollView::draw(vg, x, y, width, height, style, ctx);

  if(not isAlreadyChecked){
    this->updateModsStatus();
    isAlreadyChecked= true;
  }

}

void tab_mod_browser::setIsAlreadyChecked(bool isAlreadyChecked) {
  tab_mod_browser::isAlreadyChecked = isAlreadyChecked;
}
