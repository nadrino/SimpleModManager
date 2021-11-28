//
// Created by Nadrino on 06/06/2020.
//

#include "GameBrowserGui.h"

#include <GlobalObjects.h>
#include <ModBrowserGui.h>

GameBrowserGui::GameBrowserGui() {
  _frame_ = nullptr;
  _list_ = nullptr;
}

tsl::elm::Element *GameBrowserGui::createUI() {
// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
  _frame_ = new tsl::elm::OverlayFrame("SimpleModManager", GlobalObjects::_version_str_);

// A list that can contain sub elements and handles scrolling
  _list_ = new tsl::elm::List();

  fill_item_list();

// Return the frame to have it become the top level element of this Gui
  return _frame_;
}

void GameBrowserGui::fill_item_list() {

  _list_->clear();

  // List Items
  _list_->addItem(new tsl::elm::CategoryHeader("Folder : " + GlobalObjects::get_mod_browser().get_current_directory()));

  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {

    auto *clickableListItem = new tsl::elm::ListItem(mod_folders_list[i_folder]);
    std::string selected_folder = mod_folders_list[i_folder];
    clickableListItem->setClickListener([selected_folder](u64 keys) {
      if (keys & HidNpadButton_A) {
        tsl::changeTo<ModBrowserGui>(selected_folder);
        return true;
      }
      return false;
    });
    _list_->addItem(clickableListItem);

  }


//    _list_->addItem(new tsl::elm::ToggleListItem("Toggle List Item", true));

  // Custom Drawer, a element that gives direct access to the renderer
//    _list_->addItem(new tsl::elm::CategoryHeader("Custom Drawer", true));

//    _list_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
//      renderer->drawCircle(x + 40, y + 40, 20, true, renderer->a(0xF00F));
//      renderer->drawCircle(x + 50, y + 50, 20, true, renderer->a(0xF0F0));
//      renderer->drawRect(x + 130, y + 30, 60, 40, renderer->a(0xFF00));
//      renderer->drawString("Hello :)", false, x + 250, y + 70, 20, renderer->a(0xFF0F));
//      renderer->drawRect(x + 40, y + 90, 300, 10, renderer->a(0xF0FF));
//    }), 100);

  // Track bars
//    _list_->addItem(new tsl::elm::CategoryHeader("Track bars"));
//    _list_->addItem(new tsl::elm::TrackBar("\u2600"));
//    _list_->addItem(new tsl::elm::StepTrackBar("\uE13C", 20));
//    _list_->addItem(new tsl::elm::NamedStepTrackBar("\uE132", { "Selection 1", "Selection 2", "Selection 3" }));

  // Add the list to the frame for it to be drawn
  _frame_->setContent(_list_);

}

void GameBrowserGui::update() {

}

bool GameBrowserGui::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {

  return false;   // Return true here to singal the inputs have been consumed
}