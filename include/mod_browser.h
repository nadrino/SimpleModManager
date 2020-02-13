//
// Created by Adrien Blanchet on 03/09/2019.
//

#ifndef SWITCHTEMPLATE_BROWSER_H
#define SWITCHTEMPLATE_BROWSER_H

#include <selector.h>
#include <mod_manager.h>
#include <parameters_handler.h>

#include <string>

class mod_browser{

public:

  mod_browser();
  ~mod_browser();

  void initialize();
  void reset();

  void set_base_folder(std::string base_folder_);
  void set_max_depth(int max_depth_);
  void set_only_show_folders(bool only_show_folders_);

  int get_current_depth();
  int get_max_depth();
  std::string get_current_directory();
  std::string get_selected_entry_name();
  selector& get_selector();
  mod_manager& get_mod_manager();

  void scan_inputs(u64 kDown, u64 kHeld);
  void print_menu();
  void check_mods_status();
  bool change_directory(std::string new_directory_);
  bool go_to_selected_directory();
  bool go_back();
  int get_path_depth(std::string& path_);


private:

  bool _only_show_folders_;

  int _max_depth_;
  int _current_depth_;

  std::string _current_directory_;
  std::string _selected_entry_name_;
  std::string _base_folder_;

  selector _selector_;
  mod_manager _mod_manager_;
  parameters_handler _parameters_handler_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
