//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_GLOBALOBJECTS_H

#include <ModBrowser.h>

#include "switch.h"

#include <sstream>
#include "string"

namespace GlobalObjects {

  static ModBrowser _mod_browser_;
  extern std::string _version_str_;
  static std::string _triggerSwitchUI_;
  extern PadState gPad;

  ModBrowser& getModBrowser();

  void setTriggerSwitchUI(bool triggerSwitchUI_);
  bool doTriggerSwitchUI();

  void set_quit_now_triggered(bool value_);
  bool is_quit_now_triggered();

};

#endif //SIMPLEMODMANAGER_GLOBALOBJECTS_H
