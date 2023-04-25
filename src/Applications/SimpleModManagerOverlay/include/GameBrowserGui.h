//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GAMEBROWSERGUI_H
#define SIMPLEMODMANAGER_GAMEBROWSERGUI_H

#include "GameBrowser.h"

#include <tesla.hpp>


class GameBrowserGui : public tsl::Gui {

public:
  GameBrowserGui() = default;

  // Called when this Gui gets loaded to create the UI
  // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
  tsl::elm::Element* createUI() override;


  void update() override;

  // Called once every frame to handle inputs not handled by other UI elements
  bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;

protected:
  void fillItemList();

private:
  GameBrowser _gameBrowser_;

  tsl::elm::OverlayFrame* _frame_{nullptr};
  tsl::elm::List* _list_{nullptr};

};

#include "implementation/GameBrowserGui.impl.h"


#endif //SIMPLEMODMANAGER_GAMEBROWSERGUI_H
