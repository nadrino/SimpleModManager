//
// Created by Nadrino on 06/06/2020.
//

#include <ModBrowserGui.h>
#include <toolbox.h>
#include <GlobalObjects.h>
#include <ChangeConfigPresetGui.h>


ModBrowserGui::ModBrowserGui(const std::basic_string<char> &current_sub_folder_) {
  _current_sub_folder_ = current_sub_folder_;
  _frame_ = nullptr;
  _list_ = nullptr;
  _trigger_item_list_update_ = false;
}

tsl::elm::Element *ModBrowserGui::createUI() {

  _frame_ = new tsl::elm::OverlayFrame("SimpleModManager", GlobalObjects::_version_str_);

  std::string new_path = GlobalObjects::get_mod_browser().get_current_directory() + "/" + _current_sub_folder_;
  new_path = toolbox::remove_extra_doubled_characters(new_path, "/");

  GlobalObjects::get_mod_browser().change_directory(new_path);
  GlobalObjects::get_mod_browser().get_mod_manager().set_current_mods_folder(new_path);
  GlobalObjects::get_mod_browser().get_mods_preseter().read_parameter_file(new_path);

  // A list that can contain sub elements and handles scrolling
  fill_item_list();

  return _frame_;

}

void ModBrowserGui::fill_item_list() {

  _list_ = new tsl::elm::List();

  // List Items
  _list_->addItem(new tsl::elm::CategoryHeader(_current_sub_folder_));

  auto mods_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mods_list.size()); i_folder++) {
    auto *clickableListItem = new tsl::elm::ListItem(mods_list[i_folder]);
    std::string selected_mod_name = mods_list[i_folder];

    clickableListItem->setClickListener([selected_mod_name, this](u64 keys) {
      if (keys & KEY_A) {
        // apply mod...
        GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(selected_mod_name, true);
        GlobalObjects::get_mod_browser().get_selector().set_tag(
          GlobalObjects::get_mod_browser().get_selector().get_entry(selected_mod_name),
          GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod_name)
        );
        this->set_trigger_item_list_update(true);
        return true;
      } else if (keys & KEY_X) {
        GlobalObjects::get_mod_browser().get_mod_manager().remove_mod(selected_mod_name);
        GlobalObjects::get_mod_browser().get_selector().set_tag(
          GlobalObjects::get_mod_browser().get_selector().get_entry(selected_mod_name),
          GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod_name)
        );
        this->set_trigger_item_list_update(true);
        return true;
      }
      return false;
    });
    _list_->addItem(clickableListItem);

    double mod_fraction = GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status_fraction(mods_list[i_folder]);
    if (mod_fraction == -1) {
      mod_fraction = 0;
      _list_->addItem(
        new tsl::elm::CustomDrawer([mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
          renderer->drawRect(x, y + 4, 400, 10, renderer->a(tsl::Color(100, 100, 100, 255)));
        }), 17);
    } else {
      _list_->addItem(
        new tsl::elm::CustomDrawer([mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
          renderer->drawRect(x, y + 4, 400 * mod_fraction, 10, renderer->a(tsl::Color(100, 200, 100, 255)));
          renderer->drawRect(x + 400 * mod_fraction, y + 4, 400 * (1 - mod_fraction), 10,
                             renderer->a(tsl::Color(100, 100, 100, 255)));
        }), 17);
    }

  }

  _list_->addItem(new tsl::elm::CategoryHeader("Config Preset"));
  auto *buttonConfigPreset = new tsl::elm::ListItem(
    GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
  buttonConfigPreset->setClickListener([this](u64 keys) {
    if (keys & KEY_A) {
      // apply mod...
      this->set_trigger_item_list_update(true);
      tsl::changeTo<ChangeConfigPresetGui>();
      return true;
    }
    return false;
  });
  _list_->addItem(buttonConfigPreset);

//    _list_->addItem(new tsl::elm::NamedStepTrackBar("\uE132", { "Selection 1", "Selection 2", "Selection 3" }));

  // Add the list to the frame for it to be drawn
  _frame_->setContent(_list_); // will delete previous list
  _frame_->setFooterTitle("\uE0E1  Back     \uE0E0  Apply     \uE0E2  Disable");

}

void ModBrowserGui::update() {

  if (_trigger_item_list_update_) {
    _trigger_item_list_update_ = false;
    this->removeFocus();
    this->_list_->clear();
    fill_item_list();
  }

}

bool ModBrowserGui::handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick,
                                JoystickPosition rightJoyStick) {
  if (keysDown & KEY_B) {
    GlobalObjects::get_mod_browser().go_back();
    tsl::goBack();
    return true;
  }
  return false;   // Return true here to singal the inputs have been consumed
}

void ModBrowserGui::set_trigger_item_list_update(bool trigger_item_list_update_) {
  _trigger_item_list_update_ = trigger_item_list_update_;
}