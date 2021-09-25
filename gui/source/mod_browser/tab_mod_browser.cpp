//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "tab_mod_browser.h"
#include <GlobalObjects.h>
#include <thread>
#include <future>
#include <popup_loading.h>

#include <ext_GlobalObjects.h>

tab_mod_browser::tab_mod_browser() {

  ext_GlobalObjects::setCurrentTabModBrowserPtr(this);

  this->triggerRecheckAllMods = false;
  this->triggerUpdateModsDisplayedStatus = false;
  this->frameCounter = -1;

  // Setup the list
  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
    std::string selected_mod = mod_folders_list[i_folder];
    brls::Logger::debug("Adding mod: {}", selected_mod.c_str());
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
    item->updateActionHint(brls::Key::B, "Back");

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
    item->setValueActiveColor(nvgRGB(80, 80, 80));

    this->addView(item);
    _modsListItems_[selected_mod] = item;
  }

  if(mod_folders_list.empty()){

    auto* emptyListLabel = new brls::ListItem(
      "No mods have been found in " + GlobalObjects::get_mod_browser().get_current_directory(),
      "There you need to put your mods such as: ./<name-of-the-mod>/<file-structure-in-installed-directory>"
      );
    emptyListLabel->show([](){}, false);
    this->addView(emptyListLabel);

  }

  this->triggerRecheckAllMods = true;

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

  if(this->triggerUpdateModsDisplayedStatus){
    this->updateDisplayedModsStatus();
    this->triggerUpdateModsDisplayedStatus = false;
  }

  if(this->triggerRecheckAllMods){
    if(frameCounter == -1) frameCounter = 0;
    if(frameCounter < 1) frameCounter++; // need to keep at least one spare frame
    else{
      this->getExtModManager().start_check_all_mods();
      this->triggerRecheckAllMods = false;
      frameCounter = -1;
    }
  }

}

void tab_mod_browser::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_) {
  tab_mod_browser::triggerUpdateModsDisplayedStatus = triggerUpdateModsDisplayedStatus_;
}

void tab_mod_browser::updateDisplayedModsStatus(){

  auto* mod_manager = &GlobalObjects::get_mod_browser().get_mod_manager();

  for(auto& modItem : _modsListItems_){

    if(mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modItem.first] == modItem.second->getValue()){
      continue;
    }

    // processing tag
    std::string reference_str = mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modItem.first;
    modItem.second->setValue(
      mod_manager->get_mods_status_cache()[reference_str]
      );

    NVGcolor color;
    // processing color
    if(GlobalObjects::get_mod_browser().get_mod_manager().getModsStatusCacheFraction()[reference_str] == 0){
      color = nvgRGB(80, 80, 80);
    }
    else if(GlobalObjects::get_mod_browser().get_mod_manager().getModsStatusCacheFraction()[reference_str] == 1){
      color = nvgRGB(88, 195, 169);
    }
    else{
      color = nvgRGB(245*0.85, 198*0.85, 59*0.85);
    }
    this->_modsListItems_[modItem.first]->setValueActiveColor(color);

  }

}

