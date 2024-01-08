//
// Created by Nadrino on 06/06/2020.
//

#include <ModBrowserGui.h>
#include <Toolbox.h>

#include <ChangeConfigPresetGui.h>




ModBrowserGui::ModBrowserGui(const std::basic_string<char> &current_sub_folder_) {
  _current_sub_folder_ = current_sub_folder_;
  _frame_ = nullptr;
  _list_ = nullptr;
  _trigger_item_list_update_ = false;
}

tsl::elm::Element *ModBrowserGui::createUI() {

  _frame_ = new tsl::elm::OverlayFrame("SimpleModManager", GlobalObjects::_version_str_);

  std::string new_path = GlobalObjects::gGameBrowser.get_current_directory() + "/" + _current_sub_folder_;
  new_path = GenericToolbox::removeRepeatedCharacters(new_path, "/");

  GlobalObjects::gGameBrowser.change_directory(new_path);
  GlobalObjects::gGameBrowser.getModManager().setGameFolderPath(new_path);
  GlobalObjects::gGameBrowser.getModPresetHandler().readParameterFile(new_path);

  _list_ = new tsl::elm::List();

  // A list that can contain sub elements and handles scrolling
  fill_item_list();

  return _frame_;

}

void ModBrowserGui::fill_item_list() {

//  tsl::elm::Element* last_focused_element = this->getFocusedElement();
//  int last_focus_index = 0;
//  tsl::elm::Element* last_element;
//  int element_index = 0;
//  do{
//
//    last_element = _list_->getItemAtIndex(element_index);
//    if(last_focused_element == last_element){
//      last_focus_index = element_index;
//      break;
//    }
//    element_index++;
//
//  } while(last_element != nullptr);

  // List Items
  _list_->addItem(new tsl::elm::CategoryHeader(_current_sub_folder_));

  auto mods_list = GlobalObjects::gGameBrowser.getSelector().generateEntryTitleList();
  for (int i_folder = 0; i_folder < int(mods_list.size()); i_folder++) {
    auto *clickableListItem = new tsl::elm::ListItem(mods_list[i_folder]);
    std::string selected_mod_name = mods_list[i_folder];

    clickableListItem->setClickListener([selected_mod_name, this](u64 keys) {
      if (keys & HidNpadButton_A) {
        // apply mod...
        GlobalObjects::gGameBrowser.getModManager().applyMod(selected_mod_name, true);
        GlobalObjects::gGameBrowser.getSelector().setTag(
            GlobalObjects::gGameBrowser.getSelector().fetchEntryIndex(selected_mod_name),
            GlobalObjects::gGameBrowser.getModManager().generateStatusStr(selected_mod_name)
        );
        this->set_trigger_item_list_update(true);
        return true;
      } else if (keys & HidNpadButton_X) {
        GlobalObjects::gGameBrowser.getModManager().removeMod(selected_mod_name, 0);
        GlobalObjects::gGameBrowser.getSelector().setTag(
            GlobalObjects::gGameBrowser.getSelector().fetchEntryIndex(selected_mod_name),
            GlobalObjects::gGameBrowser.getModManager().generateStatusStr(selected_mod_name)
        );
        this->set_trigger_item_list_update(true);
        return true;
      }
      return false;
    });
    _list_->addItem(clickableListItem);

    double mod_fraction = 0; // initialize at 0
    _statusBarMap_[mods_list[i_folder]] = new tsl::elm::CustomDrawer([mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
      renderer->drawRect(x, y + 4, 400, 10, renderer->a(tsl::Color(100, 100, 100, 255)));
    });

    _list_->addItem(_statusBarMap_[mods_list[i_folder]], 17);

  }

  _list_->addItem(new tsl::elm::CategoryHeader("Config Preset"));
  auto *buttonConfigPreset = new tsl::elm::ListItem(
      GlobalObjects::gGameBrowser.getConfigHandler().get_current_config_preset_name());
  buttonConfigPreset->setClickListener([this](u64 keys) {
    if (keys & HidNpadButton_A) {
      // apply mod...
      this->set_trigger_item_list_update(true);
      tsl::changeTo<ChangeConfigPresetGui>();
      return true;
    }
    return false;
  });
  _list_->addItem(buttonConfigPreset);

  // Add the list to the frame for it to be drawn
  _frame_->setContent(_list_); // will delete previous list
  _frame_->setFooterTitle("\uE0E1  Back     \uE0E0  Apply     \uE0E2  Disable");

//  this->restoreFocus();
//  this->requestFocus(_list_->getItemAtIndex(last_focus_index), tsl::FocusDirection::None, true);

  updateModStatusBars();

}

void ModBrowserGui::updateModStatusBars(){

  auto mods_list = GlobalObjects::gGameBrowser.getSelector().generateEntryTitleList();
  for (int i_folder = 0; i_folder < int(mods_list.size()); i_folder++) {
    std::string selected_mod_name = mods_list[i_folder];
    double mod_fraction = GlobalObjects::gGameBrowser.getModManager().getModList()[i_folder].applyFraction;
    if (mod_fraction == -1) mod_fraction = 0;

    _statusBarMap_[selected_mod_name]->getMRenderFunc() = [mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
      renderer->drawRect(x, y + 4, 400 * mod_fraction, 10, renderer->a(tsl::Color(100, 200, 100, 255)));
      renderer->drawRect(x + 400 * mod_fraction, y + 4, 400 * (1 - mod_fraction), 10,
                         renderer->a(tsl::Color(100, 100, 100, 255)));
    };
  }

}

void ModBrowserGui::update() {

  if (_trigger_item_list_update_) {
    _trigger_item_list_update_ = false;
//    this->removeFocus();
//    this->_list_->clear();
//    fill_item_list();
    updateModStatusBars();
  }

}

bool ModBrowserGui::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
  if (keysDown & HidNpadButton_B) {
    GlobalObjects::getModBrowser().go_back();
    tsl::goBack();
    return true;
  }
  return false;   // Return true here to singal the inputs have been consumed
}

void ModBrowserGui::set_trigger_item_list_update(bool trigger_item_list_update_) {
  _trigger_item_list_update_ = trigger_item_list_update_;
}