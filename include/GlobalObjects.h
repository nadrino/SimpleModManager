//
// Created by Nadrino on 06/06/2020.
//

#ifndef SIMPLEMODMANAGER_GLOBALOBJECTS_H
#define SIMPLEMODMANAGER_GLOBALOBJECTS_H

#include <mod_browser.h>
#include <toolbox.h>

namespace GlobalObjects {

  static mod_browser _mod_browser_;
  static std::string _version_str_ = "v" + toolbox::get_app_version();

  mod_browser& get_mod_browser();

};


#endif //SIMPLEMODMANAGER_GLOBALOBJECTS_H
