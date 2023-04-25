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
  auto modList = this->getModManager().getModList();

  if( modList.empty() ){
    LogInfo << "No mod found." << std::endl;

    _modList_.emplace_back();
    _modList_.back().item = new brls::ListItem(
        "No mods have been found in " + this->getModManager().getGameFolderPath(),
        "There you need to put your mods such as: ./<name-of-the-mod>/<file-structure-in-installed-directory>"
    );
    _modList_.back().item->show( [](){}, false );
  }
  else{
    LogInfo << "Adding " << modList.size() << " mods..." << std::endl;

    _modList_.reserve( modList.size());
    for (const auto& mod : modList) {
      LogScopeIndent;
      LogInfo << "Adding mod: \"" << mod.modName << "\"" << std::endl;

      // memory allocation
      auto* item = new brls::ListItem(mod.modName, "", "");

      // initialization
      item->getClickEvent()->subscribe([&, mod](View* view) {
        auto* dialog = new brls::Dialog("Do you want to install \"" + mod.modName + "\" ?");

        dialog->addButton("Yes", [&, mod, dialog](brls::View* view) {
          // first, close the dialog box before the apply mod thread starts
          dialog->close();

          // starts the async routine
          _owner_->getModManager().startApplyModThread( mod.modName );
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();

        return true;
      });
      item->updateActionHint(brls::Key::A, "Apply");

      item->registerAction("Disable", brls::Key::X, [&, mod]{
        auto* dialog = new brls::Dialog("Do you want to disable \"" + mod.modName + "\" ?");

        dialog->addButton("Yes", [&, dialog, mod](brls::View* view) {
          // first, close the dialog box before the async routine starts
          dialog->close();

          // starts the async routine
          _owner_->getModManager().startRemoveModThread( mod.modName );
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();
        return true;
      });

      // create the holding struct
      _modList_.emplace_back();
      _modList_.back().mod = mod;
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

  if( _owner_->getModManager().isTriggerUpdateModsDisplayedStatus() ){
    updateDisplayedModsStatus();
    _owner_->getModManager().setTriggerUpdateModsDisplayedStatus( false );
  }

  if( this->triggerRecheckAllMods ){
    // starts the async routine
    _owner_->getModManager().startCheckAllModsThread();
    _owner_->getModManager().setTriggerUpdateModsDisplayedStatus( true );
    this->triggerRecheckAllMods = false;
  }

}

void TabModBrowser::updateDisplayedModsStatus(){
  LogDebug << __METHOD_NAME__ << std::endl;

  LogReturnIf( _modList_.size() == 1 and _modList_[0].mod.modName.empty(), "No mod in this folder. Nothing to update." );

  auto currentPreset = this->getModManager().getConfig().getCurrentPresetName();

  for( auto& modItem : _modList_ ){

    // processing tag
    modItem.item->setValue( modItem.mod.applyCache[currentPreset].statusStr );

    NVGcolor color;
    // processing color
    if     ( modItem.mod.applyCache[currentPreset].applyFraction == 0 ){
      // inactive color
      color = nvgRGB(80, 80, 80);
    }
    else if( modItem.mod.applyCache[currentPreset].applyFraction == 1 ){
      // applied color
      color = nvgRGB(88, 195, 169);
    }
    else{
      // partial color
      color = nvgRGB(
          (unsigned char) (245*0.85),
          (unsigned char) (198*0.85),
          (unsigned char) (59*0.85)
      );
    }
    modItem.item->setValueActiveColor( color );
  }

}

const ModManager& TabModBrowser::getModManager() const{
  return _owner_->getGameBrowser()->getModManager();
}
