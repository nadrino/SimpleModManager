//
// Created by Nadrino on 06/06/2020.
//

#include "ChangeConfigPresetGui.h"
#include <GlobalObjects.h>

ChangeConfigPresetGui::ChangeConfigPresetGui(){

  _question_ = "Select the config preset";
  _answers_ = GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list();

}


tsl::elm::Element* ChangeConfigPresetGui::createUI() {

  auto *rootFrame = new tsl::elm::OverlayFrame("SimpleModManager", GlobalObjects::_version_str_);

// A list that can contain sub elements and handles scrolling
  auto list = new tsl::elm::List();

// List Items
  list->addItem(new tsl::elm::CategoryHeader(_question_));

  for(int i_answer = 0 ; i_answer < int(_answers_.size()) ; i_answer++){
    auto *clickableListItem = new tsl::elm::ListItem(_answers_[i_answer]);
    std::string selected_answer = _answers_[i_answer];

    clickableListItem->setClickListener([selected_answer, this](u64 keys) {
      if (keys & KEY_A) {
        GlobalObjects::get_mod_browser().change_config_preset(selected_answer);
        tsl::goBack();
        return true;
      }
      return false;
    });
    list->addItem(clickableListItem);

    std::string install_path_display = "install-mods-base-folder: "+ GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter(
      _answers_[i_answer] + "-install-mods-base-folder"
    );
    list->addItem(new tsl::elm::CustomDrawer([install_path_display](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
      renderer->drawString(install_path_display.c_str(), false, x + 5, y + 21, 16, {255,255,255,255});
    }), 30);
  }

  rootFrame->setContent(list);

  return rootFrame;

}

bool ChangeConfigPresetGui::handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) {
  if (keysDown & KEY_B) {
    tsl::goBack();
    return true;
  }
  return false;
}