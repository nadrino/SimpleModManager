//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_GLOBALOBJECTS_H

#include <mod_browser.h>
#include <toolbox.h>
#include <sstream>

namespace GlobalObjects {

  static mod_browser _mod_browser_;
  static std::string _version_str_ = "v" + toolbox::get_app_version();
  static std::stringstream _cout_redirect_;
  static std::string _str_buffer_;

  mod_browser& get_mod_browser();
  void redirect_cout();
  void threadApplyMod(void* arg);

  void set_quit_now_triggered(bool value_);
  bool is_quit_now_triggered();

};

#endif //SIMPLEMODMANAGER_GLOBALOBJECTS_H
