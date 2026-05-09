//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "TabModBrowser.h"

#include "FrameModBrowser.h"


#include "GenericToolbox.Macro.h"
#include "GenericToolbox.String.h"
#include "Logger.h"


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
        "No mods for this game are on your SD card.",
        "Put mods in: " + this->getModManager().getGameFolderPath()
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
      item->updateActionHint(brls::Key::X, "Disable");

      item->registerAction("Delete mod", brls::Key::Y, [&, mod]{
        auto* dialog = new brls::Dialog("Do you want to delete \"" + mod.modName + "\" from the SD card and remove its installed files?");

        dialog->addButton("Yes", [&, dialog, mod](brls::View* view) {
          _focusModNameAfterDelete_ = this->getFocusTargetBeforeDelete(mod.modName);
          dialog->close();
          _owner_->getGuiModManager().startDeleteModFolderThread(mod.modName);
        });
        dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

        dialog->setCancelable(true);
        dialog->open();
        return true;
      });
      item->updateActionHint(brls::Key::Y, "Delete mod");

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

void TabModBrowser::rebuildUiFromSd(const std::string& focusModName_) {
  // Fully rebuild UI to guarantee it matches SD state.
  this->clear( true );
  _modItemList_.clear();

  auto modList = this->getModManager().getModList();
  brls::ListItem* focusItem = nullptr;

  if( modList.empty() ){
    _modItemList_.emplace_back();
    _modItemList_.back().item = new brls::ListItem(
        "No mods for this game are on your SD card.",
        "Put mods in: " + this->getModManager().getGameFolderPath()
    );
    _modItemList_.back().item->show([](){}, false);
    this->addView( _modItemList_.back().item );
    brls::Application::giveFocus( _modItemList_.back().item );
    return;
  }

  _modItemList_.reserve( modList.size() );
  for( auto& mod : modList ){
    // memory allocation
    auto* item = new brls::ListItem(mod.modName, "", "");

    // Click to install
    item->getClickEvent()->subscribe([&, mod](View* view) {
      auto* dialog = new brls::Dialog("Do you want to install \"" + mod.modName + "\" ?");

      dialog->addButton("Yes", [&, mod, dialog](brls::View* view) {
        dialog->close();
        _owner_->getGuiModManager().startApplyModThread( mod.modName );
      });
      dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });
      dialog->setCancelable(true);
      dialog->open();
      return true;
    });
    item->updateActionHint(brls::Key::A, "Apply");

    // Disable
    item->registerAction("Disable", brls::Key::X, [&, mod]{
      auto* dialog = new brls::Dialog("Do you want to disable \"" + mod.modName + "\" ?");
      dialog->addButton("Yes", [&, dialog, mod](brls::View* view) {
        dialog->close();
        _owner_->getGuiModManager().startRemoveModThread( mod.modName );
      });
      dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });
      dialog->setCancelable(true);
      dialog->open();
      return true;
    });
    item->updateActionHint(brls::Key::X, "Disable");

    // Delete folder (SD)
    item->registerAction("Delete mod", brls::Key::Y, [&, mod]{
      auto* dialog = new brls::Dialog("Do you want to delete \"" + mod.modName + "\" from the SD card and remove its installed files?");
      dialog->addButton("Yes", [&, dialog, mod](brls::View* view) {
        _focusModNameAfterDelete_ = this->getFocusTargetBeforeDelete(mod.modName);
        dialog->close();
        _owner_->getGuiModManager().startDeleteModFolderThread(mod.modName);
      });
      dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });
      dialog->setCancelable(true);
      dialog->open();
      return true;
    });
    item->updateActionHint(brls::Key::Y, "Delete mod");

    _modItemList_.emplace_back();
    _modItemList_.back().modIndex = int(_modItemList_.size()) - 1;
    _modItemList_.back().item = item;
    if( item->getLabel() == focusModName_ ) {
      focusItem = item;
    }
  }

  this->updateDisplayedModsStatus();
  for( auto& modItem : _modItemList_ ){
    this->addView( modItem.item );
  }
  this->resyncListItemFocusIndices();
  if( focusItem == nullptr && !_modItemList_.empty() ) {
    focusItem = _modItemList_.front().item;
  }
  if( focusItem != nullptr ) {
    brls::Application::giveFocus( focusItem );
  }
  this->invalidate(true);
  if( this->getParent() != nullptr ) {
    this->getParent()->invalidate(true);
  }
}

std::string TabModBrowser::getFocusTargetBeforeDelete(const std::string& modName_) const {
  for( size_t i = 0; i < _modItemList_.size(); ++i ) {
    auto* item = _modItemList_[i].item;
    if( item == nullptr || item->getLabel() != modName_ ) {
      continue;
    }

    if( i > 0 && _modItemList_[i - 1].item != nullptr ) {
      return _modItemList_[i - 1].item->getLabel();
    }
    if( i + 1 < _modItemList_.size() && _modItemList_[i + 1].item != nullptr ) {
      return _modItemList_[i + 1].item->getLabel();
    }
    return {};
  }
  return {};
}

void TabModBrowser::removeDisplayedMod(const std::string& modName_) {
  bool removed = false;
  for( size_t i = 0; i < _modItemList_.size(); ++i ) {
    auto& modItem = _modItemList_[i];
    if( modItem.item == nullptr ) {
      continue;
    }
    if( modItem.item->getLabel() == modName_ ) {
      this->removeView( static_cast<int>( i ) );
      _modItemList_.erase( _modItemList_.begin() + static_cast<long long>( i ) );
      removed = true;
      break;
    }
  }

  if( _modItemList_.empty() ) {
    auto* emptyItem = new brls::ListItem(
        "No mods for this game are on your SD card.",
        "Put mods in: " + this->getModManager().getGameFolderPath() );
    emptyItem->show([](){}, false);
    _modItemList_.push_back( ModItem{} );
    _modItemList_.back().item = emptyItem;
    this->addView( emptyItem );
  }

  if( removed ) {
    this->resyncListItemFocusIndices();
    this->invalidate(true);
    if( this->getParent() != nullptr ) {
      this->getParent()->invalidate(true);
    }
    if( not _modItemList_.empty() && _modItemList_.front().item != nullptr ) {
      brls::Application::giveFocus( _modItemList_.front().item );
    }
  }
}

void TabModBrowser::removeDisplayedMod(brls::ListItem* item_) {
  bool removed = false;
  for( size_t i = 0; i < _modItemList_.size(); ++i ) {
    auto& modItem = _modItemList_[i];
    if( modItem.item == nullptr ) {
      continue;
    }
    if( modItem.item == item_ ) {
      this->removeView( static_cast<int>( i ) );
      _modItemList_.erase( _modItemList_.begin() + static_cast<long long>( i ) );
      removed = true;
      break;
    }
  }

  if( _modItemList_.empty() ) {
    auto* emptyItem = new brls::ListItem(
        "No mods for this game are on your SD card.",
        "Put mods in: " + this->getModManager().getGameFolderPath() );
    emptyItem->show([](){}, false);
    _modItemList_.push_back( ModItem{} );
    _modItemList_.back().item = emptyItem;
    this->addView( emptyItem );
  }

  if( removed ) {
    this->resyncListItemFocusIndices();
    this->invalidate(true);
    if( this->getParent() != nullptr ) {
      this->getParent()->invalidate(true);
    }
    if( not _modItemList_.empty() && _modItemList_.front().item != nullptr ) {
      brls::Application::giveFocus( _modItemList_.front().item );
    }
  }
}

void TabModBrowser::resyncListItemFocusIndices() {
  // Borealis uses parentUserData (child index) to navigate in BoxLayout::getNextFocus().
  // After removeView(), the remaining indices can be stale and trap focus on one item.
  for( size_t i = 0; i < this->getViewsCount(); ++i ) {
    auto* child = this->getChild( i );
    if( child == nullptr ) {
      continue;
    }
    auto* parent = child->getParent();
    if( parent == nullptr ) {
      continue;
    }

    auto* userdata = static_cast<size_t*>( malloc( sizeof(size_t) ) );
    *userdata = i;
    child->setParent( parent, userdata );
  }
}

void TabModBrowser::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {

  if( _owner_->getGuiModManager().isTriggerRebuildModBrowser() ){
    LogDebug << "Rebuilding mod browser from SD..." << std::endl;
    _owner_->getGuiModManager().setTriggerRebuildModBrowser( false );
    _owner_->getGuiModManager().setTriggerUpdateModsDisplayedStatus( false );
    this->rebuildUiFromSd(_focusModNameAfterDelete_);
    _focusModNameAfterDelete_.clear();
  }

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

    // Use cached status only to avoid slow verification
    std::string statusStr;
    double frac = 0.0;

    // Check if status is cached
    if( GenericToolbox::isIn(currentPreset, modEntryList[iMod].applyCache) ){
      statusStr = modEntryList[iMod].applyCache[currentPreset].statusStr;
      frac = modEntryList[iMod].applyCache[currentPreset].applyFraction;
    } else {
      // Default to UNCHECKED if not cached
      statusStr = "UNCHECKED";
      frac = 0.0;
    }

    // processing tag
    _modItemList_[iMod].item->setValue( statusStr );

    NVGcolor color;
    // processing color
    if( GenericToolbox::startsWith(statusStr, "PARTIAL") ){
      // partial or conflicting files
      color = GenericToolbox::Borealis::orangeNvgColor;
    }
    else if( frac == 0 ){
      // inactive/unchecked color
      color = GenericToolbox::Borealis::grayNvgColor;
    }
    else if( frac == 1 ){
      // applied color (less saturated green)
      color = nvgRGB(88, 195, 169);
    }
    else{
      // partial color fallback
      color = GenericToolbox::Borealis::orangeNvgColor;
    }
    _modItemList_[iMod].item->setValueActiveColor( color );
  }
}

const ModManager& TabModBrowser::getModManager() const{
  return _owner_->getGameBrowser().getModManager();
}
