//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GAMEBROWSERGUI_H
#define SIMPLEMODMANAGER_GAMEBROWSERGUI_H

#include <tesla.hpp>


class GameBrowserGui : public tsl::Gui {
public:

  GameBrowserGui();

  // Called when this Gui gets loaded to create the UI
  // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
  tsl::elm::Element* createUI() override;

  void fill_item_list();

  // Called once every frame to update values
  void update() override;

  // Called once every frame to handle inputs not handled by other UI elements
  bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;

private:

  tsl::elm::OverlayFrame* _frame_;
  tsl::elm::List* _list_;

};


#endif //SIMPLEMODMANAGER_GAMEBROWSERGUI_H
