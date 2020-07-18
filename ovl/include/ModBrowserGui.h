//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_MODBROWSERGUI_H
#define SIMPLEMODMANAGER_MODBROWSERGUI_H

#include <tesla.hpp>

class ModBrowserGui : public tsl::Gui {
public:

  explicit ModBrowserGui(const std::basic_string<char>& current_sub_folder_);

  tsl::elm::Element* createUI() override;

  void fill_item_list();
  void updateModStatusBars();

  // Called once every frame to handle inputs not handled by other UI elements
  void update() override;
  bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override;

  void set_trigger_item_list_update(bool trigger_item_list_update_);

private:

  bool _trigger_item_list_update_;
  std::string _current_sub_folder_;
  tsl::elm::OverlayFrame* _frame_;
  tsl::elm::List* _list_;

  std::map<std::string, tsl::elm::CustomDrawer*> _statusBarMap_;

};


#endif //SIMPLEMODMANAGER_MODBROWSERGUI_H
