//
// Created by Nadrino on 03/09/2019.
//

#ifndef SWITCHTEMPLATE_BROWSER_H
#define SWITCHTEMPLATE_BROWSER_H

#include <Selector.h>
#include <ModManager.h>
#include <ParametersHandler.h>
#include <ModsPreseter.h>

#include <switch.h>

#include <string>

class ModBrowser{

public:

  ModBrowser();
  ~ModBrowser();

  void initialize();
  void reset();

  void set_base_folder(std::string base_folder_);
  void set_max_relative_depth(int max_relative_depth_);
  void set_only_show_folders(bool only_show_folders_);

  bool is_initialized();
  int get_current_relative_depth();
  int get_max_relative_depth();
  std::string get_current_directory();
  std::string get_base_folder();
  std::string get_main_config_preset();
  ParametersHandler &get_parameters_handler();
  Selector &getSelector();
  ModsPreseter &get_mods_preseter();
  ModManager &getModManager();

  void scan_inputs(u64 kDown, u64 kHeld);
  void print_menu();
  void display_conflicts_with_other_mods(const std::string &selected_mod_);
  void check_mods_status();
  bool change_directory(std::string new_directory_);
  void change_config_preset(const std::string& new_config_preset_);
  bool go_to_selected_directory();
  bool go_back();
  int get_relative_path_depth(std::string& path_);
  int get_path_depth(std::string& path_);

  uint8_t* getFolderIcon(const std::string& gameFolder_);

  void remove_all_mods(bool force_ = false);

private:

  bool _only_show_folders_;
  bool _is_initialized_;

  int _max_relative_depth_;
  int _current_relative_depth_;

  int _last_page_;
  int _last_cursor_position_;

  std::string _current_directory_;
  std::string _last_directory_;
  std::string _base_folder_;
  std::string _main_config_preset_;

  Selector _selector_;
  ModManager _mod_manager_;
  ParametersHandler _parameters_handler_;
  ModsPreseter _mods_preseter_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
