//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_GLOBALOBJECTS_H

#include <mod_browser.h>
#include <sstream>
#include "switch.h"

namespace GlobalObjects {

  static mod_browser _mod_browser_;
  extern std::string _version_str_;
  static std::string _triggerSwitchUI_;
  extern PadState gPad;

  mod_browser& get_mod_browser();

  void setTriggerSwitchUI(bool triggerSwitchUI_);
  bool doTriggerSwitchUI();

  void set_quit_now_triggered(bool value_);
  bool is_quit_now_triggered();

};

#endif //SIMPLEMODMANAGER_GLOBALOBJECTS_H
