//
// Created by Adrien BLANCHET on 30/06/2020.
//

#include "ThumbnailPresetEditor.h"
#include <GlobalObjects.h>
#include <ext_GlobalObjects.h>
#include "toolbox.h"

#include "GenericToolbox.h"

void ThumbnailPresetEditor::initialize() {

  if(_presetName_.empty()){
    this->autoAssignPresetName();
  }

  auto* modsList = new brls::List();

  auto mod_folders_list = GlobalObjects::getModBrowser().getSelector().getSelectionList();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
    std::string selected_mod = mod_folders_list[i_folder];
    auto* item = new brls::ListItem(selected_mod, "", "");

    item->getClickEvent()->subscribe([this,item](brls::View* view){
      this->getSelectedModsList().emplace_back(item->getLabel());
      std::string new_tag = item->getValue();
      if(not new_tag.empty()) new_tag += " & ";
      new_tag += "#" + std::to_string(this->getSelectedModsList().size());
      item->setValue(new_tag);

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

  if(GenericToolbox::doesElementIsInVector(_presetName_, GlobalObjects::getModBrowser().get_mods_preseter().get_presets_list())){
    _selectedModsList_ = GlobalObjects::getModBrowser().get_mods_preseter().get_mods_list(_presetName_);
  }
  this->process_tags();

}

void ThumbnailPresetEditor::process_tags() {

  // reset
  for(auto & item : itemsList){
    item->setValue("");
  }

  // set tags
  for(int i_entry = 0 ; i_entry < int(_selectedModsList_.size()) ; i_entry++){
    for(auto & item : itemsList){
      if(_selectedModsList_[i_entry] == item->getLabel()){
        std::string new_tag = item->getValue();
        if(not new_tag.empty()) new_tag += " & ";
        new_tag += "#" + std::to_string(i_entry+1);
        item->setValue(new_tag);
        break;
      }

    }
  }

  this->getSidebar()->setSubtitle(std::to_string(this->_selectedModsList_.size()) + " mods have been selected.");

}

void ThumbnailPresetEditor::setPresetName(const std::string &presetName) {
  _presetName_ = presetName;
}

std::vector<std::string> & ThumbnailPresetEditor::getSelectedModsList() {
  return _selectedModsList_;
}

void ThumbnailPresetEditor::save() {

  auto* dataHandlerPtr = &GlobalObjects::getModBrowser().get_mods_preseter().get_data_handler();
  auto* PresetsListPtr = &GlobalObjects::getModBrowser().get_mods_preseter().get_presets_list();

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

  _presetName_ = toolbox::get_user_string(_presetName_);
  (*PresetsListPtr)[preset_index] = _presetName_;

  for(int i_entry = 0 ; i_entry < int(_selectedModsList_.size()) ; i_entry++){
    (*dataHandlerPtr)[_presetName_].emplace_back(_selectedModsList_[i_entry]);
  }

  // TODO: Check for conflicts
//  show_conflicted_files(_presetName_);

  GlobalObjects::getModBrowser().get_mods_preseter().fill_selector();
  GlobalObjects::getModBrowser().get_mods_preseter().recreate_preset_file();
  GlobalObjects::getModBrowser().get_mods_preseter().read_parameter_file();

  ext_GlobalObjects::getCurrentTabModPresetPtr()->updatePresetItems();

  brls::Application::popView(brls::ViewAnimation::FADE);
  brls::Application::unblockInputs();
  cleanup();

}

void ThumbnailPresetEditor::autoAssignPresetName() {
  std::string autoName = "new-preset";
  _presetName_ = autoName;
  int count = 0;
  while(GenericToolbox::doesElementIsInVector(_presetName_, GlobalObjects::getModBrowser().get_mods_preseter().get_presets_list())){
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
