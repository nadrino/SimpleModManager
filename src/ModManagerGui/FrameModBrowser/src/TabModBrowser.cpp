//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "TabModBrowser.h"

#include "FrameModBrowser.h"


#include "GenericToolbox.Macro.h"
#include "Logger.h"


#include <future>


LoggerInit([]{
  Logger::setUserHeaderStr("[TabModBrowser]");
});


TabModBrowser::TabModBrowser(FrameModBrowser* owner_) : _owner_(owner_) {

  // Fetch the available mods
  auto modList = this->getModManager().getModList();

  if( modList.empty() ){
    LogInfo << "No mod found." << std::endl;

    _modItemList_.emplace_back();
    _modItemList_.back().item = new brls::ListItem(
        "No mods have been found in " + this->getModManager().getGameFolderPath(),
        "There you need to put your mods such as: ./<name-of-the-mod>/<file-structure-in-installed-directory>"
    );
    _modItemList_.back().item->show([](){}, false );
  }
  else{
    LogInfo << "Adding " << modList.size() << " mods..." << std::endl;

    _modItemList_.reserve(modList.size());
    for( auto& mod : modList ) {
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
          _owner_->getGuiModManager().startApplyModThread( mod.modName );
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
          _owner_->getGuiModManager().startRemoveModThread( mod.modName );
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();
        return true;
      });

      // create the holding struct
      _modItemList_.emplace_back();
      _modItemList_.back().modIndex = int(_modItemList_.size() ) - 1;
      _modItemList_.back().item = item;
    }
  }

  this->updateDisplayedModsStatus();

  // add to view
  for( auto& modItem : _modItemList_ ){
    this->addView( modItem.item );
  }

}

void TabModBrowser::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {

  ScrollView::draw(vg, x, y, width, height, style, ctx);

  if( _owner_->getGuiModManager().isTriggerUpdateModsDisplayedStatus() ){
    LogDebug << "Updating mod status display..." << std::endl;
    updateDisplayedModsStatus();
    _owner_->getGuiModManager().setTriggerUpdateModsDisplayedStatus( false );
  }
}

void TabModBrowser::updateDisplayedModsStatus(){
  LogInfo << __METHOD_NAME__ << std::endl;

  auto& modEntryList = _owner_->getGameBrowser().getModManager().getModList();
  LogReturnIf( modEntryList.empty(), "No mod in this folder. Nothing to update." );

  auto currentPreset = this->getModManager().fetchCurrentPreset().name;
  LogInfo << "Will display mod status with install preset: " << currentPreset << std::endl;

  for( size_t iMod = 0 ; iMod < modEntryList.size() ; iMod++ ){

    // processing tag
    _modItemList_[iMod].item->setValue( modEntryList[iMod].getStatus(currentPreset ) );
    double frac = modEntryList[iMod].getStatusFraction(currentPreset);

    NVGcolor color;
    // processing color
    if     ( frac == 0 ){
      // inactive color
      color = GenericToolbox::Borealis::grayNvgColor;
    }
    else if( frac == 1 ){
      // applied color (less saturated green)
      color = nvgRGB(88, 195, 169);
    }
    else{
      // partial color
      color = GenericToolbox::Borealis::orangeNvgColor;
    }
    _modItemList_[iMod].item->setValueActiveColor( color );
  }
}

const ModManager& TabModBrowser::getModManager() const{
  return _owner_->getGameBrowser().getModManager();
}
