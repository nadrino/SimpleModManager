//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "TabModBrowser.h"

#include "FrameModBrowser.h"
#include <GlobalObjects.h>
#include <PopupLoadingView.h>

#include "Logger.h"
#include "GenericToolbox.h"

#include <future>


LoggerInit([]{
  Logger::setUserHeaderStr("[TabModBrowser]");
});


TabModBrowser::TabModBrowser(FrameModBrowser* owner_) : _owner_(owner_) {

  // Fetch the available mods
  auto modFoldersList = GlobalObjects::getModBrowser().getSelector().getSelectionList();

  if( modFoldersList.empty() ){
    LogInfo << "No mod found." << std::endl;

    _modList_.emplace_back();
    _modList_.back().item = new brls::ListItem(
        "No mods have been found in " + GlobalObjects::getModBrowser().get_current_directory(),
        "There you need to put your mods such as: ./<name-of-the-mod>/<file-structure-in-installed-directory>"
    );
    _modList_.back().item->show( [](){}, false );
  }
  else{
    LogInfo << "Adding " << modFoldersList.size() << " mods..." << std::endl;

    _modList_.reserve(modFoldersList.size());
    for (const auto& selectedMod : modFoldersList) {
      LogScopeIndent;
      LogInfo << "Adding mod: \"" << selectedMod << "\"" << std::endl;

      // memory allocation
      auto* item = new brls::ListItem(selectedMod, "", "");

      // initialization
      item->getClickEvent()->subscribe([&, selectedMod](View* view) {
        auto* dialog = new brls::Dialog("Do you want to install \"" + selectedMod + "\" ?");

        dialog->addButton("Yes", [&, selectedMod, dialog](brls::View* view) {
          // first, close the dialog box before the apply mod thread starts
          dialog->close();

          // starts the async routine
          _owner_->getModManager().startApplyModThread( selectedMod );
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();

        return true;
      });
      item->updateActionHint(brls::Key::A, "Apply");

      item->registerAction("Disable", brls::Key::X, [&, selectedMod]{
        auto* dialog = new brls::Dialog("Do you want to disable \"" + selectedMod + "\" ?");

        dialog->addButton("Yes", [&, dialog, selectedMod](brls::View* view) {
          // first, close the dialog box before the async routine starts
          dialog->close();

          // starts the async routine
          _owner_->getModManager().startRemoveModThread( selectedMod );
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();
        return true;
      });

      item->setValue("Unchecked");
      item->setValueActiveColor(nvgRGB(80, 80, 80));

      // create the holding struct
      _modList_.emplace_back();
      _modList_.back().title = selectedMod;
      _modList_.back().item = item;
    }
  }

  // add to view
  for( auto& modItem : _modList_ ){
    this->addView( modItem.item );
  }

  this->triggerRecheckAllMods = true;

}

void TabModBrowser::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {

  ScrollView::draw(vg, x, y, width, height, style, ctx);

  if( _triggerUpdateModsDisplayedStatus_ ){
    updateDisplayedModsStatus();
    _triggerUpdateModsDisplayedStatus_ = false;
  }

  if(this->triggerRecheckAllMods){
    // starts the async routine
    _owner_->getModManager().startCheckAllModsThread();
    this->setTriggerUpdateModsDisplayedStatus(true);
    this->triggerRecheckAllMods = false;
  }

}

void TabModBrowser::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus_) {
  _triggerUpdateModsDisplayedStatus_ = triggerUpdateModsDisplayedStatus_;
}

void TabModBrowser::updateDisplayedModsStatus(){
  LogDebug << __METHOD_NAME__ << std::endl;

  auto* modManager = &GlobalObjects::getModBrowser().getModManager();

  LogReturnIf( _modList_.size() == 1 and _modList_[0].title.empty(), "No mod in this folder. Nothing to update." );

  for( auto& modItem : _modList_ ){
    if( modManager->get_mods_status_cache()[
          modManager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modItem.title
        ] == modItem.item->getValue()){
      continue;
    }

    // processing tag
    std::string reference_str = modManager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modItem.title;
    modItem.item->setValue( modManager->get_mods_status_cache()[reference_str] );

    NVGcolor color;
    // processing color
    if(GlobalObjects::getModBrowser().getModManager().getModsStatusCacheFraction()[reference_str] == 0){
      // inactive color
      color = nvgRGB(80, 80, 80);
    }
    else if(GlobalObjects::getModBrowser().getModManager().getModsStatusCacheFraction()[reference_str] == 1){
      // partial color
      color = nvgRGB(88, 195, 169);
    }
    else{
      // applied color
      color = nvgRGB(
          (unsigned char) (245*0.85),
          (unsigned char) (198*0.85),
          (unsigned char) (59*0.85)
      );
    }
    modItem.item->setValueActiveColor(color);
  }

}

