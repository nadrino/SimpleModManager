//
// Created by Adrien BLANCHET on 13/02/2020.
//

#ifndef SIMPLEMODMANAGER_MODS_PRESETER_H
#define SIMPLEMODMANAGER_MODS_PRESETER_H


#include <string>
#include <map>
#include "selector.h"

class mods_preseter {

public:

  mods_preseter();
  virtual ~mods_preseter();

  void initialize();
  void reset();

  int get_selected_mod_preset_index();
  std::string get_selected_mod_preset();
  std::vector<std::string> get_mods_list(std::string preset_);
  std::vector<std::string>& get_presets_list();

  void read_parameter_file(std::string mod_folder_);
  void recreate_preset_file();
  void select_mod_preset();
  void create_new_preset();
  void edit_preset(std::string preset_name_, std::vector<std::string> selected_mods_list_);

  void select_previous_mod_preset();
  void select_next_mod_preset();

protected:

  selector fill_selector();

private:

  int _selected_mod_preset_index_;
  std::string _preset_file_path_;
  std::string _mod_folder_;
  std::map<std::string, std::vector<std::string>> _data_handler_;
  std::vector<std::string> _presets_list_;

};


#endif //SIMPLEMODMANAGER_MODS_PRESETER_H
