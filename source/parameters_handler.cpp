//
// Created by Adrien Blanchet on 16/10/2019.
//

#include "parameters_handler.h"

#include "toolbox.h"
#include <iostream>
#include <fstream>
#include <sstream>

parameters_handler::parameters_handler() {

  reset();

}
parameters_handler::~parameters_handler() {

  reset();

}

void parameters_handler::initialize() {

  if(not toolbox::do_path_is_file(_parameters_file_path_)){
    recreate_parameters_file();
  }

  read_parameters();
  recreate_parameters_file(); // update version + cleanup

}
void parameters_handler::reset() {

  _parameters_file_path_ = "./parameters.ini";
  _data_handler_.clear();
  set_default_parameters();

}

void parameters_handler::set_parameters_file_path(std::string parameters_file_path_) {
  _parameters_file_path_ = parameters_file_path_;
}

std::string parameters_handler::get_parameter(std::string parameter_name_) {
  return _data_handler_[parameter_name_];
}
std::string parameters_handler::get_parameters_file_path(){
  return _parameters_file_path_;
}

void parameters_handler::set_default_parameters() {

  _data_handler_["install-mods-base-folder"] = "/atmosphere/";

}
void parameters_handler::recreate_parameters_file() {

  std::ofstream parameter_file;
  parameter_file.open (_parameters_file_path_.c_str());

  parameter_file << "# This is a config file" << std::endl;
  parameter_file << std::endl;
  parameter_file << "# base folder where mods are installed" << std::endl;
  parameter_file << "install-mods-base-folder = " << _data_handler_["install-mods-base-folder"] << std::endl;
  parameter_file << std::endl;
  parameter_file << "# DO NOT TOUCH THIS : used to recognise the last version of the program config" << std::endl;
  parameter_file << "last-program-version = " << toolbox::get_app_version() << std::endl;
  parameter_file << std::endl;

  parameter_file.close();


}
void parameters_handler::read_parameters() {

  std::ifstream parameter_file;
  parameter_file.open (_parameters_file_path_.c_str());

  std::string line;
  while( std::getline(parameter_file, line) ){

    if(line[0] == '#') continue;

    auto line_elements = toolbox::split_string(line, "=");
    if(line_elements.size() != 2) continue;

    // clean up for spaces charaters
    for(auto &element : line_elements){
      while(element[0] == ' '){
        element.erase(element.begin());
      }
      while(element[element.size()-1] == ' '){
        element.erase(element.end()-1);
      }
    }

    _data_handler_[line_elements[0]] = line_elements[1];

  }

  parameter_file.close();

}


