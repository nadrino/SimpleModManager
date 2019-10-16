//
// Created by Adrien Blanchet on 16/10/2019.
//

#ifndef SIMPLEMODMANAGER_PARAMETERS_HANDLER_H
#define SIMPLEMODMANAGER_PARAMETERS_HANDLER_H

#include <string>
#include <map>

class parameters_handler {

public:

  parameters_handler();
  ~parameters_handler();

  void initialize();
  void reset();

  void set_parameters_file_path(std::string parameters_file_path_);

  std::string get_parameter(std::string parameter_name_);
  std::string get_parameters_file_path();

protected:

  void set_default_parameters();
  void recreate_parameters_file();
  void read_parameters();


private:

  std::string _parameters_file_path_;
  std::map<std::string, std::string> _data_handler_;

};


#endif //SIMPLEMODMANAGER_PARAMETERS_HANDLER_H
