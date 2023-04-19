//
// Created by Nadrino on 03/09/2019.
//

#ifndef SWITCHTEMPLATE_BROWSER_H
#define SWITCHTEMPLATE_BROWSER_H

#include <Selector.h>
#include <ModManager.h>
#include <ConfigHandler.h>
#include <ModsPresetHandler.h>

#include <switch.h>

#include <string>


class ModBrowser{

public:
  ModBrowser() = default;

  const ConfigHandler &getConfigHandler() const;

  void set_max_relative_depth(int max_relative_depth_);
  void set_only_show_folders(bool only_show_folders_);

  int get_current_relative_depth();
  int get_max_relative_depth();
  std::string get_current_directory();
  std::string get_main_config_preset();
  ConfigHandler &get_parameters_handler();
  Selector &getSelector();
  ModManager &getModManager();
  ModsPresetHandler &getModPresetHandler();

  void scan_inputs(u64 kDown, u64 kHeld);
  void print_menu();
  void displayConflictsWithOtherMods(size_t modIndex_);
  void check_mods_status();
  bool change_directory(std::string new_directory_);
  void change_config_preset(const std::string& new_config_preset_);
  bool go_to_selected_directory();
  bool go_back();
  int get_relative_path_depth(std::string& path_);

  uint8_t* getFolderIcon(const std::string& gameFolder_);

  void removeAllMods(bool force_ = false);


  static int getPathDepth(const std::string& path_);

private:

  bool _only_show_folders_{false};

  int _max_relative_depth_{-1};
  int _current_relative_depth_;

  int _last_page_;
  int _last_cursor_position_;

  std::string _currentDirectory_{};

  std::string _last_directory_;
  std::string _main_config_preset_;

  Selector _selector_;
  ModManager _modManager_{this};
  ConfigHandler _configHandler_;
  ModsPresetHandler _modPresetHandler_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
