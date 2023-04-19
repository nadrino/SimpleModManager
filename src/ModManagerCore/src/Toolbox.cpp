//
// Created by Nadrino on 04/09/2019.
//

#include <Toolbox.h>
#include <version_config.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <cmath>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include "string"


namespace Toolbox{

  //! printout functions :
  static std::time_t _last_timestamp_;
  static double _last_displayed_value_ = -1;
  void display_loading(int current_index_, int end_index_, const std::string& title_, const std::string& prefix_, const std::string &color_str_, bool force_display_) {

    int percent = int(round(double(current_index_) / end_index_ * 100.));
    if(_last_displayed_value_ == -1) {
      set_last_timestamp();
    }
    if(
      _last_displayed_value_ == -1 or std::time(nullptr) - _last_timestamp_ >= 1 // every second
      or current_index_ == 0 // first call
      or force_display_ // display every calls
      or current_index_ >= end_index_-1 // last entry
      ){
      set_last_timestamp();
      _last_displayed_value_ = percent;
      std::stringstream ss;
      ss << prefix_ << percent << "% / " << title_;
      GenericToolbox::Switch::Terminal::printLeft(ss.str(), color_str_, true);
      consoleUpdate(nullptr);
    }

  }
  void reset_last_displayed_value(){
    _last_displayed_value_ = -1;
  }
  void set_last_timestamp(){
    _last_timestamp_ = std::time(nullptr);
  }

  //! External function
  std::string get_app_version(){
    std::stringstream ss;
    ss << get_version_major() << "." << get_version_minor() << "." << get_version_micro() << get_version_tag();
    return ss.str();
  }

}