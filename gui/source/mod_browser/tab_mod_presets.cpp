//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "tab_mod_presets.h"
#include <borealis.hpp>
#include <GlobalObjects.h>
#include <ext_GlobalObjects.h>
#include <ext_mod_manager.h>
#include <thumbnail_preset_editor.h>

tab_mod_presets::tab_mod_presets() {

  ext_GlobalObjects::setCurrentTabModPresetPtr(this);

  _maxNbPresetsSlots_ = 20;
  _nbFreeSlots_ = 0;
  _itemNewCreatePreset_ = nullptr;

  for(int i_slot = 0 ; i_slot < _maxNbPresetsSlots_ ; i_slot++){
    brls::Logger::debug("i_slot : %i",i_slot );
    _itemList_.emplace_back(new brls::ListItem( "vacant-slot" ));
    this->assignButtons(_itemList_.back(), true);
    this->addView(_itemList_.back());
  }

  _itemNewCreatePreset_ = new brls::ListItem("\uE402 Create a new mod preset");
  this->assignButtons(_itemNewCreatePreset_, false);
  this->addView(_itemNewCreatePreset_);

  this->updatePresetItems();

}

void tab_mod_presets::assignButtons(brls::ListItem *item, bool isPreset_) {

  if(isPreset_){
    item->getClickEvent()->subscribe([item](brls::View* view){});
    item->registerAction("Apply", brls::Key::A, [item]{
      auto* dialog = new brls::Dialog("Do you want disable all mods and apply the preset \"" + item->getLabel() + "\" ?");

      dialog->addButton("Yes", [dialog, item](brls::View* view) {
        if(ext_GlobalObjects::getCurrentTabModBrowserPtr() != nullptr){
          ext_mod_manager::setOnCallBackFunction([dialog](){dialog->close();});
          ext_GlobalObjects::getCurrentTabModBrowserPtr()->getExtModManager().start_apply_mod_preset(item->getLabel());
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
    item->registerAction("Remove", brls::Key::X, [this, item]{

      auto* dialog = new brls::Dialog("Do you want to delete the preset \"" + item->getLabel() + "\" ?");

      dialog->addButton("Yes", [this, item, dialog](brls::View* view) {
        GlobalObjects::get_mod_browser().get_mods_preseter().delete_mod_preset(item->getLabel());
//        this->setTriggerUpdate(true);
        this->updatePresetItems();
        dialog->close();
      });
      dialog->addButton("No", [dialog](brls::View* view) {
        dialog->close();
      });

      dialog->setCancelable(true);
      dialog->open();
      return true;

    });
    item->updateActionHint(brls::Key::X, "Remove");
    item->registerAction("Edit", brls::Key::Y, [item]{
      // open editor
      auto* editor = new thumbnail_preset_editor();
      editor->setPresetName(item->getLabel());
      editor->initialize();

      auto* icon = ext_GlobalObjects::getCurrentFrameModBrowserPtr()->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open("Preset Editor", icon, 0x20000, editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }
      else{
        brls::PopupFrame::open("Preset Editor", editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }

      return true;
    });
    item->updateActionHint(brls::Key::Y, "Edit");
  }
  else {
    // new preset
    item->getClickEvent()->subscribe([this](brls::View* view){

      if(this->getNbFreeSlots() <= 0){
        brls::Application::notify("No available slots");
        return true;
      }

      // create new preset
      auto* editor = new thumbnail_preset_editor();
      editor->initialize();

      auto* icon = ext_GlobalObjects::getCurrentFrameModBrowserPtr()->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open("Preset Editor", icon, 0x20000, editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }
      else{
        brls::PopupFrame::open("Preset Editor", editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }

      return true;
    });
    item->updateActionHint(brls::Key::A, "Create");

  }

}

void tab_mod_presets::updatePresetItems() {

  auto presets_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_presets_list();
  for(int i_preset = 0 ; i_preset < int(presets_list.size()) ; i_preset++){
    if(i_preset+1 >= _maxNbPresetsSlots_){ // should not
      break;
    }
    auto mods_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(presets_list[i_preset]);
    this->_itemList_[i_preset]->setLabel(presets_list[i_preset]);
    this->_itemList_[i_preset]->setValue(std::to_string(mods_list.size()) + " mods in this set");
    this->_itemList_[i_preset]->expand(true);
//    this->_itemList_[i_preset]->show([](){});
  }

  // collapsing all hidden slots
  _nbFreeSlots_ = 0;
  for(int i_hidden = int(presets_list.size()) ; i_hidden < _maxNbPresetsSlots_ ; i_hidden++){
    this->_itemList_[i_hidden]->collapse(true);
//    this->_itemList_[i_hidden]->hide([](){});
    _nbFreeSlots_++;
  }

  brls::Application::unblockInputs();
//  if(brls::Application::getCurrentFocus() != nullptr and brls::Application::getCurrentFocus()->isCollapsed()){
//    brls::Application::getCurrentFocus()->onFocusLost();
//    brls::Application::giveFocus(_itemNewCreatePreset_->getDefaultFocus());
//  }

}

int tab_mod_presets::getNbFreeSlots() const {
  return _nbFreeSlots_;
}


