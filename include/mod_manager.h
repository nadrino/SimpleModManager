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

  void set_log_file_path(std::string log_file_path_);
  void set_install_mods_base_folder(std::string install_mods_base_folder_);

  std::string get_install_mods_base_folder();

  std::string get_mod_status(std::string mod_folder_path_);
  void apply_mod(std::string mod_folder_path_, bool force_ = false);
  void remove_mod(std::string mod_folder_path_);
  void display_mod_files_status(std::string mod_folder_path_);

protected:

  std::string ask_to_replace(std::string path_);

private:

  std::string _install_mods_base_folder_;
  std::string _log_file_path_;
  std::map<std::string, std::vector<std::string>> _files_list_cache_;

  parameters_handler _parameters_handler_;



};


#endif //MODAPPLIER_MOD_MANAGER_H
