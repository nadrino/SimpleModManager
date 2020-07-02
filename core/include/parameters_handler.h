//
// Created by Nadrino on 16/10/2019.
//

#ifndef SIMPLEMODMANAGER_PARAMETERS_HANDLER_H
#define SIMPLEMODMANAGER_PARAMETERS_HANDLER_H

#include <string>
#include <vector>
#include <map>

class parameters_handler {

public:

  parameters_handler();
  ~parameters_handler();

  void initialize();
  void reset();

  void set_parameters_file_path(std::string parameters_file_path_);
  void set_current_config_preset_id(int selected_preset_id_);
  void set_current_config_preset_name(std::string preset_name_);
  void set_parameter(std::string parameter_name_, std::string value_);

  int get_current_config_preset_id();
  std::string get_parameter(std::string parameter_name_);
  std::string get_parameters_file_path();
  std::string get_current_config_preset_name();

  std::vector<std::string> & get_presets_list();

  void increment_selected_preset_id();

protected:

  void set_default_parameters();
  void recreate_parameters_file();
  void read_parameters();

  void fill_current_preset_parameters();


private:

  int _current_config_preset_id_;
  std::string _parameters_file_path_;
  std::vector<std::string> _presets_list_;
  std::map<std::string, std::string> _data_handler_;

};


#endif //SIMPLEMODMANAGER_PARAMETERS_HANDLER_H
