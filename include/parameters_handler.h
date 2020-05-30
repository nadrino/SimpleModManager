//
// Created by Adrien Blanchet on 16/10/2019.
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
  void set_selected_preset_id(int selected_preset_id_);
  void set_selected_preset(std::string preset_name_);

  std::string get_parameter(std::string parameter_name_);
  std::string get_parameters_file_path();
  std::string get_selected_install_preset_name();

  void increment_selected_preset_id();

protected:

  void set_default_parameters();
  void recreate_parameters_file();
  void read_parameters();

  void append_to_preset_list(std::string preset_);
  void fill_current_preset_parameters();


private:

  int _selected_preset_id_;
  std::string _parameters_file_path_;
  std::vector<std::string> _presets_list_;
  std::map<std::string, std::string> _data_handler_;

};


#endif //SIMPLEMODMANAGER_PARAMETERS_HANDLER_H
