//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "TabModBrowser.h"
#include <GlobalObjects.h>
#include <PopupLoading.h>
#include <ext_GlobalObjects.h>

#include "Logger.h"

#include <thread>
#include <future>

LoggerInit([]{
  Logger::setUserHeaderStr("[TabModBrowser]");
});

TabModBrowser::TabModBrowser() {

  GuiGlobalObjects::setCurrentTabModBrowserPtr(this);

  this->triggerRecheckAllMods = false;
  this->triggerUpdateModsDisplayedStatus = false;
  this->frameCounter = -1;

  // Setup the list
  auto modFoldersList = GlobalObjects::getModBrowser().getSelector().getSelectionList();
  for (const auto& selectedMod : modFoldersList) {
    LogDebug << "Adding mod: \"" << selectedMod << "\"" << std::endl;
    auto* item = new brls::ListItem(selectedMod, "", "");
    item->getClickEvent()->subscribe([this, selectedMod](View* view) {
      auto* dialog = new brls::Dialog("Do you want to install \"" + selectedMod + "\" ?");

      dialog->addButton("Yes", [selectedMod, dialog](brls::View* view) {
        if(GuiGlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
          GuiModManager::setOnCallBackFunction([dialog](){dialog->close();});
          GuiGlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().setModName(selectedMod);
          GuiGlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_apply_mod();
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

    item->registerAction("Disable", brls::Key::X, [selectedMod]{

      auto* dialog = new brls::Dialog("Do you want to disable \"" + selectedMod + "\" ?");

      dialog->addButton("Yes", [selectedMod, dialog](brls::View* view) {
        if(GuiGlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
          GuiModManager::setOnCallBackFunction([dialog](){dialog->close();});
          GuiGlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().setModName(selectedMod);
          GuiGlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_remove_mod();
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
    _modsListItems_[selectedMod] = item;
  }

  if(modFoldersList.empty()){

    auto* emptyListLabel = new brls::ListItem(
      "No mods have been found in " + GlobalObjects::getModBrowser().get_current_directory(),
      "There you need to put your mods such as: ./<name-of-the-mod>/<file-structure-in-installed-directory>"
      );
    emptyListLabel->show([](){}, false);
    this->addView(emptyListLabel);

  }

  this->triggerRecheckAllMods = true;

}

GuiModManager &TabModBrowser::getExtModManager() {
  return _extModManager_;
}

std::map<std::string, brls::ListItem *> &TabModBrowser::getModsListItems() {
  return _modsListItems_;
}

void TabModBrowser::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
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

void TabModBrowser::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_) {
  TabModBrowser::triggerUpdateModsDisplayedStatus = triggerUpdateModsDisplayedStatus_;
}

void TabModBrowser::updateDisplayedModsStatus(){

  auto* mod_manager = &GlobalObjects::getModBrowser().get_mod_manager();

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
    if(GlobalObjects::getModBrowser().get_mod_manager().getModsStatusCacheFraction()[reference_str] == 0){
      color = nvgRGB(80, 80, 80);
    }
    else if(GlobalObjects::getModBrowser().get_mod_manager().getModsStatusCacheFraction()[reference_str] == 1){
      color = nvgRGB(88, 195, 169);
    }
    else{
      color = nvgRGB(245*0.85, 198*0.85, 59*0.85);
    }
    this->_modsListItems_[modItem.first]->setValueActiveColor(color);

  }

}

