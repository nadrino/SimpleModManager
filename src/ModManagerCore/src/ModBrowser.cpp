//
// Created by Nadrino on 03/09/2019.
//

#include <ModBrowser.h>
#include <Toolbox.h>
#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <algorithm>
#include <utility>
#include <cstring>

ModBrowser::ModBrowser(){

  reset();

}
ModBrowser::~ModBrowser() { reset(); }

void ModBrowser::initialize(){

  _selector_.setMaxItemsPerPage(30);

  _parameters_handler_.initialize();
  _main_config_preset_ = _parameters_handler_.get_current_config_preset_name();

  set_base_folder(_parameters_handler_.get_parameter("stored-mods-base-folder"));
  _modManager_.set_install_mods_base_folder(_parameters_handler_.get_parameter("install-mods-base-folder"));
  _modManager_.set_parameters_handler_ptr(&_parameters_handler_);
  _modManager_.initialize();

  change_directory(_base_folder_);

  _is_initialized_ = true;

}
void ModBrowser::reset(){

  _is_initialized_ = false;

  _max_relative_depth_ = -1;
  _base_folder_ = "/";
  _currentDirectory_ = _base_folder_;
  _only_show_folders_ = false;
  _main_config_preset_ = _parameters_handler_.get_current_config_preset_name();

  _modManager_.reset();
  _parameters_handler_.reset();
  _selector_ = Selector();

}

void ModBrowser::set_base_folder(std::string base_folder_){
  _base_folder_ = std::move(base_folder_);
}
void ModBrowser::set_max_relative_depth(int max_relative_depth_){
  _max_relative_depth_ = max_relative_depth_;
}
void ModBrowser::set_only_show_folders(bool only_show_folders_){
  _only_show_folders_ = only_show_folders_;
}

int ModBrowser::get_current_relative_depth(){
  return _current_relative_depth_;
}
int ModBrowser::get_max_relative_depth(){
  return _max_relative_depth_;
}
std::string ModBrowser::get_current_directory(){
  return _currentDirectory_;
}
std::string ModBrowser::get_base_folder(){
  return _base_folder_;
}
std::string ModBrowser::get_main_config_preset(){
  return _main_config_preset_;
}
ParametersHandler &ModBrowser::get_parameters_handler(){
  return _parameters_handler_;
}
Selector &ModBrowser::getSelector(){
  return _selector_;
}
ModsPresetHandler &ModBrowser::getModPresetHandler(){
  return _modPresetHandler_;
}
ModManager &ModBrowser::getModManager(){
  return _modManager_;
}

void ModBrowser::scan_inputs(u64 kDown, u64 kHeld){

  _selector_.scanInputs(kDown, kHeld);

  if(kDown == 0 and kHeld == 0) return;

  if(get_current_relative_depth() == get_max_relative_depth()){
    if     (kDown & HidNpadButton_A){ // select folder / apply mod
      // make mod action (ACTIVATE OR DEACTIVATE)
      _modManager_.applyMod(_selector_.getSelectedEntryTitle());

      GenericToolbox::Switch::Terminal::printLeft("Checking...", GenericToolbox::ColorCodes::magentaBackground, true);
      _selector_.setTag(
          _selector_.getCursorPosition(),
          _modManager_.getModStatus(_selector_.getSelectedEntryTitle())
      );

    }
    else if(kDown & HidNpadButton_X){ // disable mod
      _modManager_.removeMod(_selector_.getSelectedEntryTitle());
      _selector_.setTag(
          _selector_.getCursorPosition(),
          _modManager_.getModStatus(_selector_.getSelectedEntryTitle())
      );
    }
    else if(kDown & HidNpadButton_Y){ // mod detailed infos
      std::string display_mod_files_status_str ="Show the status of each mod files";
      std::string display_mod_files_conflicts_str = "Show the list of conflicts";
      auto answer = Selector::ask_question(
        "Mod options:",
        {
          display_mod_files_status_str,
          display_mod_files_conflicts_str
        }
      );
      if(answer == display_mod_files_status_str){
        _modManager_.display_mod_files_status(get_current_directory() + "/" + _selector_.getSelectedEntryTitle());
      }
      else if(answer == display_mod_files_conflicts_str){
        displayConflictsWithOtherMods( _selector_.getCursorPosition() );
      }
    }
    else if(kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR){ // recheck all mods
      std::string recheck_all_mods_answer = "Reset mods status cache and recheck all mods";
      std::string disable_all_mods_answer = "Disable all mods";
      std::string set_install_preset_answer = "Attribute a config preset for this folder";
      auto menu_answer = Selector::ask_question(
        "Options for this folder:",
        {
          recheck_all_mods_answer,
          disable_all_mods_answer,
          set_install_preset_answer
        }
      );

      if(menu_answer == recheck_all_mods_answer){
        auto answer = Selector::ask_question("Do you which to recheck all mods ?",
                                            std::vector<std::string>({"Yes", "No"}));
        if(answer == "Yes"){
          _modManager_.resetAllModsCacheStatus();
          check_mods_status();
        }
      }
      else if(menu_answer == disable_all_mods_answer){
        removeAllMods();
        check_mods_status();
      }
      else if(menu_answer == set_install_preset_answer){

        std::vector<std::string> config_presets_list;
        std::vector<std::vector<std::string>> config_presets_description_list;

        std::string null_preset="Keep the main menu preset (default).";
        config_presets_list.emplace_back(null_preset);
        config_presets_description_list.emplace_back(std::vector<std::string>());

        for(const auto& preset_name: _parameters_handler_.get_presets_list()){
          config_presets_list.emplace_back(preset_name);
          config_presets_description_list.emplace_back(std::vector<std::string>());
          config_presets_description_list.back().emplace_back(
            "install-mods-base-folder: "+_parameters_handler_.get_parameter(
              preset_name+"-install-mods-base-folder"
            )
          );
        }

        auto answer = Selector::ask_question("Please select the config preset you want for this folder:",
                                            config_presets_list, config_presets_description_list);

        // overwriting
        std::string this_folder_config_file_path = _currentDirectory_ + "/this_folder_config.txt";
        GenericToolbox::deleteFile(this_folder_config_file_path);
        if(answer != null_preset){
          GenericToolbox::dumpStringInFile(this_folder_config_file_path, answer);
          change_config_preset(answer);
        }
        else{
          // restore the config preset
          change_config_preset(_main_config_preset_);
        }

      }

    }
    else if(kDown & HidNpadButton_Minus){ // Enter the mods preset menu a mods preset
      _modPresetHandler_.selectModPreset();
    }
    else if(kDown & HidNpadButton_Plus){ // Apply a mods preset
      std::string answer = Selector::ask_question(
          "Do you want to apply " + _modPresetHandler_.getSelectedModPresetName() + " ?",
        std::vector<std::string>({"Yes", "No"})
      );
      if(answer == "Yes"){
        removeAllMods(true);
        const std::vector<std::string>& modList = _modPresetHandler_.getSelectedPresetModList();
        _modManager_.applyModList(modList);
        check_mods_status();
      }
    }
    else if(kDown & HidNpadButton_L){
      _modPresetHandler_.getSelector().selectPrevious();
    }
    else if(kDown & HidNpadButton_R){
      _modPresetHandler_.getSelector().selectNextEntry();
    }
  }
  else {
    if(kDown & HidNpadButton_A){ // select folder / apply mod
      go_to_selected_directory();
      if(get_current_relative_depth() == get_max_relative_depth()){
        _modManager_.setCurrentModsFolder(_currentDirectory_);
        check_mods_status();
        _modPresetHandler_.setModFolder(_currentDirectory_ );
      }
    }
    else if(kDown & HidNpadButton_Y){ // switch between config preset
      if(get_current_relative_depth() == 0){
        _parameters_handler_.increment_selected_preset_id();
        _modManager_.set_install_mods_base_folder(
          _parameters_handler_.get_parameter("install-mods-base-folder")
          );
      }
    }
    else if(kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR){ // switch between config preset
      auto answer = Selector::ask_question(
        "Do you want to switch back to the GUI ?",
        std::vector<std::string>({"Yes", "No"})
      );
      if(answer == "Yes") {
        get_parameters_handler().set_parameter("use-gui", "1");
        GlobalObjects::set_quit_now_triggered(true);
      }
    }
  }

  if(kDown & HidNpadButton_B){ // back
    if(not go_back()){
      std::cout << "Can't go back." << std::endl;
    }
  }

  print_menu();

}
void ModBrowser::print_menu(){
  consoleClear();
  GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());

  // ls
  GenericToolbox::Switch::Terminal::printLeft("Current Folder : " + _currentDirectory_, GenericToolbox::ColorCodes::redBackground);
  std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
  _selector_.print();
  std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());

  std::cout << "  Page (" << _selector_.getCursorPage() + 1 << "/" << _selector_.getNbPages() << ")" << std::endl;
  std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
  if(get_current_relative_depth() == get_max_relative_depth())
    GenericToolbox::Switch::Terminal::printLeft("Mod preset : " + _modPresetHandler_.getSelectedModPresetName());
  GenericToolbox::Switch::Terminal::printLeft(
      std::string("Configuration preset : ")
      + GenericToolbox::ColorCodes::greenBackground
      + _parameters_handler_.get_current_config_preset_name()
      + GenericToolbox::ColorCodes::resetColor
    );
  GenericToolbox::Switch::Terminal::printLeft("install-mods-base-folder = " + _modManager_.get_install_mods_base_folder());
  std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
  if(get_current_relative_depth() == get_max_relative_depth()){
    GenericToolbox::Switch::Terminal::printLeftRight(" ZL : Rescan all mods", "ZR : Disable all mods ");
    GenericToolbox::Switch::Terminal::printLeftRight(" A/X : Apply/Disable mod", "L/R : Previous/Next preset ");
    GenericToolbox::Switch::Terminal::printLeftRight(" -/+ : Select/Apply mod preset", "Y : Mod options ");
  }
  else{
    GenericToolbox::Switch::Terminal::printLeftRight(" A : Select folder", "Y : Change config preset ");
    GenericToolbox::Switch::Terminal::printLeftRight(" B : Quit", "ZL/ZR : Switch back to the GUI ");
  }
  if(get_current_relative_depth() > 0) GenericToolbox::Switch::Terminal::printLeft(" B : Go back");
//  toolbox::print_left_right(
//    GenericToolbox::Switch::Hardware::getMemoryUsageStr(GenericToolbox::Switch::Hardware::Applet),
//    GenericToolbox::Switch::Hardware::getMemoryUsageStr(GenericToolbox::Switch::Hardware::System),
//    toolbox::green_bg
//    );
  consoleUpdate(nullptr);

}
void ModBrowser::displayConflictsWithOtherMods(size_t modIndex_){

  consoleClear();

  struct ModFileConflict{
    std::string file{};
    std::vector<std::string> conflictingModList{};
  };


  auto fileList = GenericToolbox::getListOfFilesInSubFolders(
      _modManager_.getCurrentModFolderPath() + "/" + _selector_.getSelectedEntryTitle()
  );
  std::vector<ModFileConflict> modFileList;

  // create entries for files of the given mod
  modFileList.reserve( fileList.size() );
  for( auto& file : fileList ){
    modFileList.emplace_back();
    modFileList.back().file = file;
  }

  // now loop over all the other mods to find collisions


  std::vector<std::string> sel_list;
  std::vector<std::vector<std::string>> mods_conflict_files_list;
  for(auto& conflict: conflicts){
    if(conflict.second.empty()) continue;

    sel_list.emplace_back(conflict.first);
    mods_conflict_files_list.emplace_back(std::vector<std::string>());
    for(auto &conflict_file_path : conflict.second){
      mods_conflict_files_list.back().emplace_back("  | " + conflict_file_path);
    }
  }

  Selector sel;
  sel.set_selection_list(sel_list);
  sel.set_description_list(mods_conflict_files_list);

  sel.set_max_items_per_page(GenericToolbox::Switch::Hardware::getTerminalHeight() - 9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      GenericToolbox::Switch::Terminal::printLeft("Conflicts with " + selected_mod_, GenericToolbox::ColorCodes::redBackground);
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      sel.print();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("Page (" + std::to_string(sel.get_current_page() + 1) + "/" + std::to_string(sel.get_nb_pages()) + ")");
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" B : Go back", "");
      if(sel.get_nb_pages() > 1) GenericToolbox::Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
      consoleUpdate(nullptr);
    }

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);
    kHeld = padGetButtons(&GlobalObjects::gPad);

    if (kDown & HidNpadButton_B) {
      break; // break in order to return to hbmenu
    }

    sel.scan_inputs(kDown, kHeld);

  }




}
void ModBrowser::check_mods_status(){
  if(get_current_relative_depth() != get_max_relative_depth()) return;
  _selector_.clearTags();
  for( size_t iMod = 0 ; iMod < _selector_.getEntryList().size() ; iMod++ ){
    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    if(kDown & HidNpadButton_B) break;

    _selector_.setTag(iMod, "Checking...");
    print_menu();
    std::stringstream ss;
    ss << "Checking (" << iMod + 1 << "/" << _selector_.getEntryList().size() << ") : ";
    ss << _selector_.getEntryList()[iMod].title << "...";
    GenericToolbox::Switch::Terminal::printLeft(ss.str(), GenericToolbox::ColorCodes::magentaBackground);
    consoleUpdate(nullptr);
    _selector_.setTag(iMod, _modManager_.getModStatus(_selector_.getEntryList()[iMod].title));
  }
}
bool ModBrowser::change_directory(std::string new_directory_){

  // removing trailing /
  if(new_directory_.size() != 1 and new_directory_[new_directory_.size()-1] == '/'){ // remove '/' tail unless "/" is the whole path
    new_directory_ = new_directory_.substr(0, new_directory_.size()-1);
  }

  // sanity check
  if(not GenericToolbox::doesPathIsFolder(new_directory_)) return false;
  auto new_path_relative_depth = get_relative_path_depth(new_directory_);
  if(new_path_relative_depth != -1 and new_path_relative_depth > _max_relative_depth_) return false;

  // update objects
  int restored_cursor_position = -1;
  int restored_page = -1;
  if(new_directory_ == _last_directory_){
    restored_cursor_position = _last_cursor_position_;
    restored_page = _last_page_;
  }
  _last_directory_ = _currentDirectory_;
  _last_cursor_position_ = long( _selector_.getCursorPosition() );
  _last_page_ = _selector_.getCursorPage();
  _currentDirectory_ = new_directory_;
  _current_relative_depth_ = new_path_relative_depth;

  // update list of entries
  std::vector<std::string> selection_list;
  if(not _only_show_folders_) { selection_list = GenericToolbox::getListOfEntriesInFolder(_currentDirectory_); }
  else                        { selection_list = GenericToolbox::getListOfSubFoldersInFolder(_currentDirectory_); }

  selection_list.erase(std::remove(selection_list.begin(), selection_list.end(), ".plugins"), selection_list.end());
  std::sort(selection_list.begin(), selection_list.end());
  _selector_.setEntryList(selection_list);

  // restoring cursor position
  if(restored_page != -1){
    while(_selector_.getCurrentPage() != restored_page){
      _selector_.next_page();
    }
    _selector_.moveCursorPosition(restored_cursor_position);
  }

  if(new_path_relative_depth == _max_relative_depth_){
    std::string thisFolderConfigFilePath = _currentDirectory_ + "/this_folder_config.txt";
    if(GenericToolbox::doesPathIsFile(thisFolderConfigFilePath)){
      _main_config_preset_ = _parameters_handler_.get_current_config_preset_name(); // backup
      auto vector_this_folder_config = GenericToolbox::dumpFileAsVectorString(thisFolderConfigFilePath);
      if(not vector_this_folder_config.empty()){ this->change_config_preset(vector_this_folder_config[0]); }
    }
  }
  else{
    // restore initial config preset
    if(_main_config_preset_ != _parameters_handler_.get_current_config_preset_name()){
      change_config_preset(_main_config_preset_);
    }
  }

  return true;
}
void ModBrowser::change_config_preset(const std::string& new_config_preset_){

  _parameters_handler_.set_current_config_preset_name(new_config_preset_);
  _modManager_.set_install_mods_base_folder(
    _parameters_handler_.get_parameter("install-mods-base-folder")
  );

}
bool ModBrowser::go_to_selected_directory(){
  std::string new_path = _currentDirectory_ + "/" + _selector_.getSelectedEntryTitle();
  new_path = GenericToolbox::removeRepeatedCharacters(new_path, "/");
  return change_directory(new_path);
}
bool ModBrowser::go_back(){

  if(get_relative_path_depth(_currentDirectory_) <= 0 or _currentDirectory_ == "/") return false; // already at maximum root

  auto folder_elements = GenericToolbox::splitString(_currentDirectory_, "/");
  auto new_path = "/" + GenericToolbox::joinVectorString(folder_elements, "/", 0, folder_elements.size()-1);
  new_path = GenericToolbox::removeRepeatedCharacters(new_path, "/");
  if(not GenericToolbox::doesPathIsFolder(new_path)){
    std::cerr << "Can't go back, \"" << new_path << "\" is not a folder" << std::endl;
    return false;
  }
  return change_directory(new_path);

}
int ModBrowser::get_relative_path_depth(std::string& path_){
  int path_depth = getPathDepth(path_);
  int base_depth = getPathDepth(_base_folder_);
  int relative_path_depth = path_depth - base_depth;
  return relative_path_depth;
}

uint8_t* ModBrowser::getFolderIcon(const std::string& gameFolder_){
  uint8_t* icon = nullptr;

  if(get_current_relative_depth() == get_max_relative_depth()-1){
    std::string game_folder_path = get_current_directory() + "/" + gameFolder_;
    icon = GenericToolbox::Switch::Utils::getFolderIconFromTitleId(GenericToolbox::Switch::Utils::lookForTidInSubFolders(game_folder_path));
  }

  return icon;
}

void ModBrowser::removeAllMods(bool force_){
  std::string answer;
  GenericToolbox::Switch::IO::p.useCrcCheck = false;
  if(not force_){
    answer = Selector::ask_question(
      "Do you want to disable all mods ?",
      std::vector<std::string>({"Yes", "No"})
    );
  } else{
    answer = "Yes";
  }
  if(answer == "Yes") {
    for( auto& mod : _selector_.getEntryList() ){
      _modManager_.removeMod( mod.title );
    }
  }
  GenericToolbox::Switch::IO::p.useCrcCheck = true;
}

int ModBrowser::getPathDepth(const std::string& path_){
  int pathDepth = int((GenericToolbox::splitString(path_, "/")).size() );
  if(GenericToolbox::doesStringStartsWithSubstring(path_, "/")) pathDepth--;
  if(GenericToolbox::doesStringEndsWithSubstring(path_, "/")) pathDepth--;
  return pathDepth;
}
