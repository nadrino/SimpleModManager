//
// Created by Adrien Blanchet on 06/09/2019.
//

#ifndef MODAPPLIER_MOD_MANAGER_H
#define MODAPPLIER_MOD_MANAGER_H

#include <parameters_handler.h>

#include <vector>
#include <string>
#include <map>

class mod_manager {

public:

  mod_manager();
  ~mod_manager();

  void initialize();

  void set_install_mods_base_folder(std::string install_mods_base_folder_);

  std::string get_install_mods_base_folder();

  void set_current_mods_folder(std::string folder_path_);
  void load_mods_status_cache_file();
  void save_mods_status_cache_file();
  void reset_mod_cache_status(std::string mod_name_);
  void reset_all_mods_cache_status();

  std::string get_mod_status(std::string mod_name_);

  void apply_mod(std::string mod_name_, bool force_ = false);
  void remove_mod(std::string mod_name_);
  void display_mod_files_status(std::string mod_folder_path_);

protected:

  std::string ask_to_replace(std::string path_);

private:

  std::string _install_mods_base_folder_;
  std::string _current_mods_folder_path_;
  std::map<std::string, std::vector<std::string>> _relative_file_path_list_cache_;
  std::map<std::string, std::string> _mods_status_cache_;

  parameters_handler _parameters_handler_;



};


#endif //MODAPPLIER_MOD_MANAGER_H
