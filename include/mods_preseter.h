//
// Created by Adrien BLANCHET on 13/02/2020.
//

#ifndef SIMPLEMODMANAGER_MODS_PRESETER_H
#define SIMPLEMODMANAGER_MODS_PRESETER_H


#include <string>

class mods_preseter {

public:

  mods_preseter();
  virtual ~mods_preseter();

  void read_parameter_file(std::string mod_folder_);

};


#endif //SIMPLEMODMANAGER_MODS_PRESETER_H
