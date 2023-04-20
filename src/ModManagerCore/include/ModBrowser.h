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

  // setters


  // getters
  const ConfigHandler &getConfigHandler() const;
  Selector &getSelector();
  ModManager &getModManager();
  ModsPresetHandler &getModPresetHandler();


  std::string get_current_directory();
  std::string get_main_config_preset();
  ConfigHandler &get_parameters_handler();

  // browse
  void selectGame(const std::string &gameName_);

  // IO
  void scanInputs(u64 kDown, u64 kHeld);
  void printConsole();
  void displayConflictsWithOtherMods(size_t modIndex_);



  void check_mods_status();
  void change_config_preset(const std::string& new_config_preset_);
  bool goToGameDirectory();
  bool go_back();
  int get_relative_path_depth(std::string& path_);

  uint8_t* getFolderIcon(const std::string& gameFolder_);

  void removeAllMods(bool force_ = false);


  static int getPathDepth(const std::string& path_);

private:
  bool _isGameSelected_{false};

  int _last_page_;
  int _last_cursor_position_;

  std::string _currentDirectory_{};

  std::string _last_directory_;
  std::string _main_config_preset_;

  Selector _gameSelector_;
  Selector _modSelector_;
  ModManager _modManager_{this};
  ConfigHandler _configHandler_;
  ModsPresetHandler _modPresetHandler_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
