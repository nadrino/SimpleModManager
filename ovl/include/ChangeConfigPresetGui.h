//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_CHANGECONFIGPRESETGUI_H
#define SIMPLEMODMANAGER_CHANGECONFIGPRESETGUI_H

#include <tesla.hpp>

class ChangeConfigPresetGui : public tsl::Gui {
public:

  explicit ChangeConfigPresetGui();

  tsl::elm::Element* createUI() override;

  // Called once every frame to handle inputs not handled by other UI elements
  bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override;

private:

  std::string _question_;
  std::vector<std::string> _answers_;

};


#endif //SIMPLEMODMANAGER_CHANGECONFIGPRESETGUI_H
