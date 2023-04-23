//
// Created by Adrien BLANCHET on 30/06/2020.
//

#include "ThumbnailPresetEditor.h"
#include "FrameModBrowser.h"

#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"

#include "sstream"


void ThumbnailPresetEditor::initialize() {

  if(_presetName_.empty()){
    this->autoAssignPresetName();
  }

  auto* modsList = new brls::List();

  auto modFolderList = GlobalObjects::gGameBrowser.getSelector().generateEntryTitleList();
  for( auto& modFolder : modFolderList ){
    auto* item = new brls::ListItem(modFolder, "", "");

    item->getClickEvent()->subscribe([this,item](brls::View* view){
      this->getSelectedModsList().emplace_back(item->getLabel());

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
      this->process_tags();
      return true;
    });

    modsList->addView(item);

    // disable + to quit
    item->registerAction("", brls::Key::PLUS, []{return true;}, true);
//    item->updateActionHint(brls::Key::PLUS, ""); // make the change visible

    itemsList.emplace_back(item);
  }

  this->setContentView(modsList);

  this->getSidebar()->getButton()->getClickEvent()->subscribe([this](brls::View* view){
    this->save();
    return true;
  });
  this->getSidebar()->registerAction("", brls::Key::PLUS, []{return true;}, true);
  this->getSidebar()->setTitle(_presetName_);

  this->registerAction("", brls::Key::PLUS, []{return true;}, true);
//  this->updateActionHint(brls::Key::PLUS, ""); // make the change visible

  if(GenericToolbox::doesElementIsInVector(_presetName_,
                                           GlobalObjects::gGameBrowser.getModPresetHandler().getPresetsList())){
    _selectedModsList_ = GlobalObjects::gGameBrowser.getModPresetHandler().getModsList(_presetName_);
  }
  this->process_tags();

}

void ThumbnailPresetEditor::process_tags() {

  // reset
  for(auto & item : itemsList){
    item->setValue("");
  }

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

void ThumbnailPresetEditor::setPresetName(const std::string &presetName_) {
  _presetName_ = presetName_;
}

std::vector<std::string> & ThumbnailPresetEditor::getSelectedModsList() {
  return _selectedModsList_;
}

void ThumbnailPresetEditor::save() {

  auto& dataHandler = GlobalObjects::gGameBrowser.getModPresetHandler().getDataHandler();
  auto* PresetsListPtr = &GlobalObjects::gGameBrowser.getModPresetHandler().getPresetsList();

  auto& modsList = GlobalObjects::gGameBrowser.getModPresetHandler().getModsList(_presetName_ );

  (*dataHandlerPtr)[_presetName_].clear();
  (*dataHandlerPtr)[_presetName_].resize(0);

  int preset_index = -1;
  for(int i_index = 0 ; i_index < int((*PresetsListPtr).size()) ; i_index++){
    if((*PresetsListPtr)[i_index] == _presetName_) preset_index = i_index;
  }

  if(preset_index == -1){
    preset_index = (*PresetsListPtr).size();
    (*PresetsListPtr).emplace_back(_presetName_);
  }

  _presetName_ = GenericToolbox::Switch::UI::openKeyboardUi(_presetName_);
  (*PresetsListPtr)[preset_index] = _presetName_;

  for(int i_entry = 0 ; i_entry < int(_selectedModsList_.size()) ; i_entry++){
    (*dataHandlerPtr)[_presetName_].emplace_back(_selectedModsList_[i_entry]);
  }

  // TODO: Check for conflicts
//  showConflictingFiles(_presetName_);

  GlobalObjects::gGameBrowser.getModPresetHandler().fillSelector();
  GlobalObjects::gGameBrowser.getModPresetHandler().writeConfigFile();
  GlobalObjects::gGameBrowser.getModPresetHandler().readParameterFile();

  _owner_->getTabModPresets()->setTriggerUpdateItem( true );

  brls::Application::popView(brls::ViewAnimation::FADE);
  brls::Application::unblockInputs();
  cleanup();

}

void ThumbnailPresetEditor::autoAssignPresetName() {
  std::string autoName = "new-preset";
  _presetName_ = autoName;
  int count = 0;
  while( GenericToolbox::doesElementIsInVector(
      _presetName_, GlobalObjects::gGameBrowser.getModPresetHandler().getPresetsList())
      ){
    _presetName_ = autoName + "-" + std::to_string(count);
    count++;
  }
}

void ThumbnailPresetEditor::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                                 brls::FrameContext *ctx) {

  if(_selectedModsList_.empty()){
    this->getSidebar()->getButton()->setState(brls::ButtonState::DISABLED);
  }
  else if(this->getSidebar()->getButton()->getState() == brls::ButtonState::DISABLED and not _selectedModsList_.empty()){
    this->getSidebar()->getButton()->setState(brls::ButtonState::ENABLED);
  }

  AppletFrame::draw(vg, x, y, width, height, style, ctx);
}

void ThumbnailPresetEditor::cleanup() {

  itemsList.resize(0);
  _selectedModsList_.resize(0);

}
