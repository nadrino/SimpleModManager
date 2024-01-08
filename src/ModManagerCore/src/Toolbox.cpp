//
// Created by Nadrino on 04/09/2019.
//

#include <Toolbox.h>
#include <version_config.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include "string"


namespace Toolbox{
  //! External function
  std::string getAppVersion(){
    std::stringstream ss;
    ss << get_version_major() << "." << get_version_minor() << "." << get_version_micro() << get_version_tag();
    return ss.str();
  }

}