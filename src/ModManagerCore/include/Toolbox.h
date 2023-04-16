//
// Created by Nadrino on 04/09/2019.
//

#ifndef SIMPLEMODMANAGER_TOOLBOX_H
#define SIMPLEMODMANAGER_TOOLBOX_H

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <switch.h>

namespace Toolbox{

  //! printout/printin functions :
  void display_loading(int current_index_, int end_index_, const std::string& title_, const std::string& prefix_,
    const std::string &color_str_ = "", bool force_display_ = false);

  //! toolbox vars management functions :
  void reset_last_displayed_value();

  void set_last_timestamp();
  void set_CRC_check_is_enabled(bool CRC_check_is_enabled_);
  bool get_CRC_check_is_enabled();

  //! External function
  std::string get_app_version();
}

#endif //SIMPLEMODMANAGER_TOOLBOX_H
