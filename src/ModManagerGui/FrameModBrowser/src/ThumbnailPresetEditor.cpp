//
// Created by Adrien BLANCHET on 30/06/2020.
//

#include "ThumbnailPresetEditor.h"
#include "FrameModBrowser.h"

#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"

#include "sstream"



ThumbnailPresetEditor::ThumbnailPresetEditor(FrameModBrowser* owner_) : _owner_(owner_){

  // the list that will appear
  auto* modList = new brls::List();

  // fill the items
  for( auto& modFolder : _owner_->getGameBrowser().getModManager().getModList() ){
    auto* item = new brls::ListItem(modFolder.modName, "", "");

    item->getClickEvent()->subscribe([this,item](brls::View* view){
      _selectedModsList_.emplace_back( item->getLabel() );

      std::stringstream ss;
      ss << item->getValue();
      if( not ss.str().empty() ) ss << " & ";
      ss << "#" << this->getSelectedModsList().size();
      item->setValue( ss.str() );

      return true;
    });

    item->registerAction("Remove", brls::Key::X, [this,item]{
      int original_size = this->getSelectedModsList().size();
      // in decreasing order because we want to remove the last occurrence of the mod first
      for(int i_entry = int(this->getSelectedModsList().size()) - 1 ; i_entry >= 0 ; i_entry--){
        if(item->getLabel() == this->getSelectedModsList()[i_entry]){
          this->getSelectedModsList().erase(this->getSelectedModsList().begin() + i_entry);
          break; // for loop
        }
      }
      if(original_size > 0) this->getSelectedModsList().resize(original_size-1); // even if nothing has been selected remove one element
      this->updateTags();
      return true;
    });

    modList->addView(item);

    // disable + to quit
    item->registerAction("", brls::Key::PLUS, []{ return true; }, true);
//    item->updateActionHint(brls::Key::PLUS, ""); // make the change visible

    itemsList.emplace_back(item);
  }

  this->setContentView(modList);

  this->getSidebar()->getButton()->getClickEvent()->subscribe([this](brls::View* view){
    this->save();
    return true;
  });
  this->getSidebar()->registerAction("", brls::Key::PLUS, []{return true;}, true);

  // make sure the default preset name is unique
  this->autoAssignPresetName();

  this->registerAction("", brls::Key::PLUS, []{return true;}, true);
//  this->updateActionHint(brls::Key::PLUS, ""); // make the change visible

  int presetIndex{ GenericToolbox::findElementIndex(_presetName_, _owner_->getGameBrowser().getModPresetHandler().getPresetList(),
                                                    [](const PresetData& p ){ return p.name; })};
  if( presetIndex != -1 ){
    _selectedModsList_ = _owner_->getGameBrowser().getModPresetHandler().getPresetList()[presetIndex].modList;
  }
  this->updateTags();
}


void ThumbnailPresetEditor::setPresetName(const std::string &presetName_) {
  _presetName_ = presetName_;
  this->getSidebar()->setTitle( presetName_ );
}

std::vector<std::string> & ThumbnailPresetEditor::getSelectedModsList() {
  return _selectedModsList_;
}

void ThumbnailPresetEditor::updateTags() {

  // reset tags
  for(auto & item : itemsList){  item->setValue(""); }

  // set tags
  for( size_t iEntry = 0 ; iEntry < _selectedModsList_.size() ; iEntry++ ){
    for(auto & item : itemsList){

      if( _selectedModsList_[iEntry] != item->getLabel() ) continue;

      std::stringstream ss;
      ss << item->getValue();
      if( not ss.str().empty() ) ss << " & ";
      ss << "#" << iEntry + 1;
      item->setValue( ss.str() );

      break;
    }
  }

  std::stringstream ss;
  ss << this->_selectedModsList_.size() << " mods have been selected.";
  this->getSidebar()->setSubtitle( ss.str() );

}
void ThumbnailPresetEditor::save() {

  auto& presetList = _owner_->getGameBrowser().getModPresetHandler().getPresetList();

  int presetIndex{ GenericToolbox::findElementIndex(
      _presetName_, presetList, [](const PresetData& p ){ return p.name; }
  )};

  PresetData* presetSlotPtr{nullptr};
  if( presetIndex == -1 ){
    // create new
    presetList.emplace_back();
    presetSlotPtr = &presetList.back();
    presetSlotPtr->name = _presetName_;
  }
  else{
    // edit existing
    presetSlotPtr = &presetList[presetIndex];
    presetSlotPtr->modList.clear();
  }

  // let us edit the name
  _presetName_ = GenericToolbox::Switch::UI::openKeyboardUi(_presetName_);
  presetSlotPtr->name = _presetName_;

  presetSlotPtr->modList.reserve( _selectedModsList_.size() );
  for( auto& mod : _selectedModsList_ ){
    presetSlotPtr->modList.emplace_back( mod );
  }

  // TODO: Check for conflicts
//  showConflictingFiles(_presetName_);

  _owner_->getGameBrowser().getModPresetHandler().writeConfigFile();
  _owner_->getGameBrowser().getModPresetHandler().readConfigFile();

  _owner_->getTabModPresets()->setTriggerUpdateItem( true );

  brls::Application::popView(brls::ViewAnimation::FADE);
  brls::Application::unblockInputs();
  cleanup();

}
void ThumbnailPresetEditor::autoAssignPresetName() {
  std::string autoName = "new-preset";
  _presetName_ = autoName;
  int count{0};
  while( GenericToolbox::doesElementIsInVector(
      _presetName_, _owner_->getGameBrowser().getModPresetHandler().getPresetList(),
      [](const PresetData& p ){ return p.name; }
  )){
    _presetName_ = autoName + "-" + std::to_string(count);
    count++;
  }
  this->setPresetName( _presetName_ );
}
void ThumbnailPresetEditor::cleanup() {
  itemsList.clear();
  _selectedModsList_.clear();
}

void ThumbnailPresetEditor::draw(
    NVGcontext *vg, int x, int y,
    unsigned int width, unsigned int height,
    brls::Style *style, brls::FrameContext *ctx) {

  if     ( _selectedModsList_.empty() ){
    // save button should be hidden if nothing is selected
    this->getSidebar()->getButton()->setState(brls::ButtonState::DISABLED);
  }
  else if( this->getSidebar()->getButton()->getState() == brls::ButtonState::DISABLED ){
    // re-enable it
    this->getSidebar()->getButton()->setState(brls::ButtonState::ENABLED);
  }

  // trigger the default draw
  this->ThumbnailFrame::draw(vg, x, y, width, height, style, ctx);
//  this->AppletFrame::draw(vg, x, y, width, height, style, ctx);
}

