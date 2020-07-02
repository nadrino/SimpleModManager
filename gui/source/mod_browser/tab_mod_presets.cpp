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

  _triggerUpdate_ = false;

  ext_GlobalObjects::setCurrentTabModPresetPtr(this);

  auto presets_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_presets_list();
  for(const auto& preset_name : presets_list){
    _itemList_.emplace_back(this->createNewPresetItem(preset_name));
    this->addView(_itemList_.back());
  }

  _itemList_.emplace_back(new brls::ListItem("\uE402 Create a new mod preset"));
  this->assignButtons(_itemList_.back(), false);
  this->addView(_itemList_.back());

}


std::vector<brls::ListItem *> &tab_mod_presets::getItemList() {
  return _itemList_;
}

void tab_mod_presets::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                           brls::FrameContext *ctx) {

  if(_triggerUpdate_){
    this->updatePresetItems();
    _triggerUpdate_ = false;
  }

  ScrollView::draw(vg, x, y, width, height, style, ctx);
}

void tab_mod_presets::setTriggerUpdate(bool triggerUpdate) {
  _triggerUpdate_ = triggerUpdate;
}

brls::ListItem *tab_mod_presets::createNewPresetItem(const std::string& presetName_) {

  auto mods_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(presetName_);

  auto* item = new brls::ListItem( presetName_,"","" );
  item->setValue(std::to_string(mods_list.size()) + " mods in this set");

  this->assignButtons(item, true);

  return item;
}

void tab_mod_presets::assignButtons(brls::ListItem *item, bool isPreset_) {

  if(isPreset_){
    item->getClickEvent()->subscribe([item](brls::View* view){});
    item->registerAction("Apply", brls::Key::A, [this, item]{
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
        this->setTriggerUpdate(true);
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
  else{
    // new preset
    item->getClickEvent()->subscribe([](brls::View* view){});
    item->updateActionHint(brls::Key::A, "Create");
    item->registerAction("", brls::Key::A, [](){
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
    }, false);

    item->updateActionHint(brls::Key::X, "");
    item->registerAction("", brls::Key::X, []{return true;}, true);
    item->updateActionHint(brls::Key::Y, "");
    item->registerAction("", brls::Key::Y, []{return true;}, true);
  }

}

void tab_mod_presets::updatePresetItems() {

  // it will be overwritten if a preset has been added
  auto newPresetLabel = this->_itemList_.back()->getLabel();

  // get the new presets list
  auto presets_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_presets_list();
  for(int i_preset = 0 ; i_preset < int(presets_list.size()) ; i_preset++){
    auto mods_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(presets_list[i_preset]);

    // overwrites labels -> if one is added, it will replace the "Create a new Preset" item
    if(i_preset < int(this->_itemList_.size())){
      this->_itemList_[i_preset]->setLabel(presets_list[i_preset]);
      this->_itemList_[i_preset]->setValue(std::to_string(mods_list.size()) + " mods in this set");
      this->assignButtons(this->_itemList_[i_preset], true);
    }
    // need to create one more slot (in practice it should not be used since only 1 preset can be created at a time)
    else{
      this->_itemList_.emplace_back(this->createNewPresetItem(presets_list[i_preset]));
      this->_itemList_.back()->setValue(std::to_string(mods_list.size()) + " mods in this set");
      this->assignButtons(this->_itemList_[i_preset], true);
      this->addView(this->_itemList_.back());
    }

    if(this->_itemList_[i_preset]->isCollapsed()){
      this->_itemList_[i_preset]->expand(true);
    }
  }

  // in the case where one preset has been added -> need to add one more slot for the "create a new preset" item
  if(this->_itemList_.back()->getLabel() != newPresetLabel){
    _itemList_.emplace_back(new brls::ListItem(newPresetLabel));
    this->assignButtons(_itemList_.back(), false);
    this->addView(_itemList_.back());
  }

  // in the case where one preset has been deleted
  // _itemList_ will be too big -> need to hide
  int lastItemDisplayedIndex = int(_itemList_.size()) - 1; // assume all items are displayed (which may not be the case -> double check)
  while(lastItemDisplayedIndex > int(presets_list.size())){ // should be equal
    _itemList_[lastItemDisplayedIndex]->collapse(true);
    lastItemDisplayedIndex--;
  }

  // make sure the last item is create new preset
  if(_itemList_[lastItemDisplayedIndex]->isCollapsed()){
    this->_itemList_[lastItemDisplayedIndex]->expand(true);
  }
  _itemList_[lastItemDisplayedIndex]->setLabel(newPresetLabel);
  _itemList_[lastItemDisplayedIndex]->setValue("");
  this->assignButtons(_itemList_[lastItemDisplayedIndex], false);



}


