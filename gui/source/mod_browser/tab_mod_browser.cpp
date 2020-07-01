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

      auto* dialog = new brls::Dialog("Do you want to install \"" + selected_mod + "\" ?");

      dialog->addButton("Yes", [selected_mod, dialog](brls::View* view) {
        if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
          ext_mod_manager::setOnCallBackFunction([dialog](){dialog->close();});
          ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().setModName(selected_mod);
          ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_apply_mod();
        }
      });
      dialog->addButton("No", [dialog](brls::View* view) {
        dialog->close();
      });

      dialog->setCancelable(true);
      dialog->open();

      return true;
    });
    item->updateActionHint(brls::Key::A, "Apply");

    item->registerAction("Disable", brls::Key::X, [selected_mod]{

      auto* dialog = new brls::Dialog("Do you want to disable \"" + selected_mod + "\" ?");

      dialog->addButton("Yes", [selected_mod, dialog](brls::View* view) {
        if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
          ext_mod_manager::setOnCallBackFunction([dialog](){dialog->close();});
          ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().setModName(selected_mod);
          ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_remove_mod();
        }
      });
      dialog->addButton("No", [dialog](brls::View* view) {
        dialog->close();
      });

      dialog->setCancelable(true);
      dialog->open();
      return true;
    });

    item->setValue("Unchecked");

    this->addView(item);
    _modsListItems_[selected_mod] = item;
  }

}

void tab_mod_browser::updateModsStatus() {

  brls::Logger::debug("updateModsStatus");
  this->getExtModManager().start_check_all_mods();

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
