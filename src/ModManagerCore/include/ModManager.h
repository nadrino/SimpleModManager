//
// Created by Nadrino on 06/09/2019.
//

#ifndef MODAPPLIER_MOD_MANAGER_H
#define MODAPPLIER_MOD_MANAGER_H

#include <ParametersHandler.h>

#include <vector>
#include <string>
#include <map>

class ModManager {

public:

  ModManager();
  ~ModManager();

  void initialize();
  void reset();

  void set_install_mods_base_folder(std::string install_mods_base_folder_);
  void set_use_cache_only_for_status_check(bool use_cache_only_for_status_check_);
  void set_ignored_file_list(std::vector<std::string>& ignored_file_list_);

  std::string get_install_mods_base_folder();
  std::string & getCurrentModFolderPath();
  std::vector<std::string>& get_ignored_file_list();
  std::map<std::string, std::string> & getModsStatusCache();
  bool isUseCacheOnlyForStatusCheck();
  std::map<std::string, double> &getModsStatusCacheFraction();
  ParametersHandler *getParametersHandlerPtr();

  void set_parameters_handler_ptr(ParametersHandler *parameters_handler_ptr_);
  void setCurrentModsFolder(const std::string &folder_path_);
  void load_mods_status_cache_file();
  void save_mods_status_cache_file();
  void reset_mod_cache_status(std::string mod_name_);
  void resetAllModsCacheStatus();

  double get_mod_status_fraction(std::string mod_name_);
  std::string get_mod_status(std::string mod_name_);

  void applyMod(const std::string& mod_name_, bool force_ = false);
  void applyModList(const std::vector<std::string> &modNamesList_);
  void remove_mod(std::string mod_name_);
  void display_mod_files_status(std::string mod_folder_path_);

protected:

  std::string ask_to_replace(std::string path_);

private:

  bool _use_cache_only_for_status_check_;
  bool _internal_parameters_handler_;

  std::string _install_mods_base_folder_;
  std::string _current_mods_folder_path_;
  std::vector<std::string> _ignored_file_list_;
//  std::map<std::string, std::vector<std::string>> _relative_file_path_list_cache_;
  std::map<std::string, std::string> _mods_status_cache_;
  std::map<std::string, double> _mods_status_cache_fraction_;

  ParametersHandler* _parameters_handler_ptr_;




};


#endif //MODAPPLIER_MOD_MANAGER_H
