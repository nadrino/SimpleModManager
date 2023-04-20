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


using namespace GenericToolbox;

// setters


// getters
ModManager &ModBrowser::getModManager(){
  return _modManager_;
}
const ConfigHandler &ModBrowser::getConfigHandler() const {
  return _configHandler_;
}

// Browse
void ModBrowser::selectGame(const std::string &gameName_) {
  if( _isGameSelected_ ){
    GenericToolbox::Switch::Terminal::makePause( "SHOULD GET THERE: " + __METHOD_NAME__ );
    return;
  }

  // reset the mod selector
  _modSelector_ = Selector();

  // list mods
  auto modList = GenericToolbox::getListOfSubFoldersInFolder(
      _configHandler_.getConfig().baseFolder + "/" + gameName_
  );

  _modManager_.setGameFolderPath( _configHandler_.getConfig().baseFolder + "/" + gameName_ );
}


std::string ModBrowser::get_current_directory(){
  return _currentDirectory_;
}
std::string ModBrowser::get_main_config_preset(){
  return _main_config_preset_;
}
ConfigHandler &ModBrowser::get_parameters_handler(){
  return _configHandler_;
}
Selector &ModBrowser::getSelector(){
  return _gameSelector_;
}
ModsPresetHandler &ModBrowser::getModPresetHandler(){
  return _modPresetHandler_;
}

void ModBrowser::scanInputs(u64 kDown, u64 kHeld){

  // nothing to do?
  if( kDown == 0 and kHeld == 0 ){ return; }

  Selector* currentSelector{ &_gameSelector_ };
  if( _isGameSelected_ ){ currentSelector = &_modSelector_; }

  // forward to the selector
  currentSelector->scanInputs(kDown, kHeld );


  if( _isGameSelected_ ){
    if     (kDown & HidNpadButton_A){ // select folder / apply mod
      // make mod action (ACTIVATE OR DEACTIVATE)
      _modManager_.applyMod( currentSelector->getSelectedEntryTitle() );

      Switch::Terminal::printLeft("Checking...", ColorCodes::magentaBackground, true);
      currentSelector->setTag(
          currentSelector->getCursorPosition(),
          _modManager_.generateStatusStr( currentSelector->getSelectedEntryTitle() )
      );

    }
    else if(kDown & HidNpadButton_X){
      // disable mod
      _modManager_.removeMod( currentSelector->getSelectedEntryTitle() );
      currentSelector->setTag(
          currentSelector->getCursorPosition(),
          _modManager_.generateStatusStr(currentSelector->getSelectedEntryTitle())
      );
    }
    else if(kDown & HidNpadButton_Y){ // mod detailed infos
      std::string display_mod_files_status_str ="Show the status of each mod files";
      std::string display_mod_files_conflicts_str = "Show the list of conflicts";
      auto answer = Selector::askQuestion(
          "Mod options:",
          {
              display_mod_files_status_str,
              display_mod_files_conflicts_str
          }
      );
      if(answer == display_mod_files_status_str){
        _modManager_.displayModFilesStatus(currentSelector->getSelectedEntryTitle() );
      }
      else if(answer == display_mod_files_conflicts_str){
        displayConflictsWithOtherMods(currentSelector->getCursorPosition() );
      }
    }
    else if(kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR){ // recheck all mods
      std::string recheck_all_mods_answer = "Reset mods status cache and recheck all mods";
      std::string disable_all_mods_answer = "Disable all mods";
      std::string set_install_preset_answer = "Attribute a config preset for this folder";
      auto menu_answer = Selector::askQuestion(
          "Options for this folder:",
          {
              recheck_all_mods_answer,
              disable_all_mods_answer,
              set_install_preset_answer
          }
      );

      if(menu_answer == recheck_all_mods_answer){
        auto answer = Selector::askQuestion("Do you which to recheck all mods ?",
                                            std::vector<std::string>({"Yes", "No"}));
        if(answer == "Yes"){
          _modManager_.resetAllModsCacheAndFile();
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

        for(const auto& preset_name: _configHandler_.get_presets_list()){
          config_presets_list.emplace_back(preset_name);
          config_presets_description_list.emplace_back(std::vector<std::string>());
          config_presets_description_list.back().emplace_back(
              "install-mods-base-folder: " + _configHandler_.get_parameter(
              preset_name+"-install-mods-base-folder"
            )
          );
        }

        auto answer = Selector::askQuestion("Please select the config preset you want for this folder:",
                                            config_presets_list, config_presets_description_list);

        // overwriting
        std::string this_folder_config_file_path = _currentDirectory_ + "/this_folder_config.txt";
        deleteFile(this_folder_config_file_path);
        if(answer != null_preset){
          dumpStringInFile(this_folder_config_file_path, answer);
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
      std::string answer = Selector::askQuestion(
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
      this->goToGameDirectory();
      _modManager_.setGameFolderPath(_currentDirectory_);
      this->check_mods_status();
      _modPresetHandler_.setModFolder(_currentDirectory_ );
    }
    else if(kDown & HidNpadButton_Y){ // switch between config preset
      if(get_current_relative_depth() == 0){
        _configHandler_.selectNextPreset();
        _modManager_.set_install_mods_base_folder(
            _configHandler_.get_parameter("install-mods-base-folder")
          );
      }
    }
    else if(kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR){ // switch between config preset
      auto answer = Selector::askQuestion(
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

}
void ModBrowser::printConsole(){

  Selector* currentSelector{ &_gameSelector_ };
  if( _isGameSelected_ ){ currentSelector = &_modSelector_; }

  currentSelector->clearMenu();
  currentSelector->getHeader() >> "SimpleModManager v" + Toolbox::getAppVersion() << std::endl;
  currentSelector->getHeader() << ColorCodes::redBackground << "Current Folder : " + _currentDirectory_ << ColorCodes::resetColor << std::endl;
  currentSelector->getHeader() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;

  currentSelector->getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  currentSelector->getFooter() << "  Page (" << currentSelector->getCursorPage() + 1 << "/" << currentSelector->getNbPages() << ")" << std::endl;
  currentSelector->getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  if( _isGameSelected_ ){
    currentSelector->getFooter() << "Mod preset : " << _modPresetHandler_.getSelectedModPresetName() << std::endl;
  }
  currentSelector->getFooter() << "Configuration preset : " << ColorCodes::greenBackground;
  currentSelector->getFooter() << _configHandler_.getConfig().getCurrentPresetName() << ColorCodes::resetColor << std::endl;
  currentSelector->getFooter() << "install-mods-base-folder = " + _configHandler_.getConfig().getCurrentPreset().installBaseFolder << std::endl;
  currentSelector->getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  if( _isGameSelected_ ){
    currentSelector->getFooter() << " ZL : Rescan all mods" >> "ZR : Disable all mods " << std::endl;
    currentSelector->getFooter() << " A/X : Apply/Disable mod" >> "L/R : Previous/Next preset " << std::endl;
    currentSelector->getFooter() << " -/+ : Select/Apply mod preset" >> "Y : Mod options " << std::endl;
    currentSelector->getFooter() << " B : Go back" << std::endl;
  }
  else{
    currentSelector->getFooter() << " A : Select folder" >> "Y : Change config preset " << std::endl;
    currentSelector->getFooter() << " B : Quit" >> "ZL/ZR : Switch back to the GUI " << std::endl;
  }

  currentSelector->printTerminal();

}
void ModBrowser::displayConflictsWithOtherMods(size_t modIndex_){

  consoleClear();

  struct ModFileConflict{
    std::string file{};
    std::vector<std::string> conflictingModList{};
  };


  auto fileList = getListOfFilesInSubFolders(
      _modManager_.getCurrentModFolderPath() + "/" + _modSelector_.getSelectedEntryTitle()
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

  sel.set_max_items_per_page(Switch::Hardware::getTerminalHeight() - 9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      Switch::Terminal::printLeft("Conflicts with " + selected_mod_, ColorCodes::redBackground);
      std::cout << repeatString("*", Switch::Hardware::getTerminalWidth());
      sel.printTerminal();
      std::cout << repeatString("*", Switch::Hardware::getTerminalWidth());
      Switch::Terminal::printLeft("Page (" + std::to_string(sel.get_current_page() + 1) + "/" + std::to_string(sel.get_nb_pages()) + ")");
      std::cout << repeatString("*", Switch::Hardware::getTerminalWidth());
      Switch::Terminal::printLeftRight(" B : Go back", "");
      if(sel.get_nb_pages() > 1) Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
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
  _gameSelector_.clearTags();
  for(size_t iMod = 0 ; iMod < _gameSelector_.getEntryList().size() ; iMod++ ){
    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    if(kDown & HidNpadButton_B) break;

    _gameSelector_.setTag(iMod, "Checking...");
    printConsole();
    std::stringstream ss;
    ss << "Checking (" << iMod + 1 << "/" << _gameSelector_.getEntryList().size() << ") : ";
    ss << _gameSelector_.getEntryList()[iMod].title << "...";
    Switch::Terminal::printLeft(ss.str(), ColorCodes::magentaBackground);
    consoleUpdate(nullptr);
    _gameSelector_.setTag(iMod, _modManager_.generateStatusStr(_gameSelector_.getEntryList()[iMod].title));
  }
}
void ModBrowser::change_config_preset(const std::string& new_config_preset_){

  _configHandler_.selectPresetWithName(new_config_preset_);
  _modManager_.set_install_mods_base_folder(
      _configHandler_.get_parameter("install-mods-base-folder")
  );

}
bool ModBrowser::goToGameDirectory(){
  std::string new_path = _currentDirectory_ + "/" + _gameSelector_.getSelectedEntryTitle();
  new_path = removeRepeatedCharacters(new_path, "/");
  return change_directory(new_path);
}
bool ModBrowser::go_back(){

  if(get_relative_path_depth(_currentDirectory_) <= 0 or _currentDirectory_ == "/") return false; // already at maximum root

  auto folder_elements = splitString(_currentDirectory_, "/");
  auto new_path = "/" + joinVectorString(folder_elements, "/", 0, folder_elements.size()-1);
  new_path = removeRepeatedCharacters(new_path, "/");
  if(not doesPathIsFolder(new_path)){
    std::cerr << "Can't go back, \"" << new_path << "\" is not a folder" << std::endl;
    return false;
  }
  return change_directory(new_path);

}
int ModBrowser::get_relative_path_depth(std::string& path_){
  int path_depth = getPathDepth(path_);
  int base_depth = getPathDepth( _configHandler_.getConfig().baseFolder );
  int relative_path_depth = path_depth - base_depth;
  return relative_path_depth;
}

uint8_t* ModBrowser::getFolderIcon(const std::string& gameFolder_){
  uint8_t* icon = nullptr;

  if(get_current_relative_depth() == get_max_relative_depth()-1){
    std::string game_folder_path = get_current_directory() + "/" + gameFolder_;
    icon = Switch::Utils::getFolderIconFromTitleId(Switch::Utils::lookForTidInSubFolders(game_folder_path));
  }

  return icon;
}

void ModBrowser::removeAllMods(bool force_){
  std::string answer;
  Switch::IO::p.useCrcCheck = false;
  if(not force_){
    answer = Selector::askQuestion(
        "Do you want to disable all mods ?",
        std::vector<std::string>({"Yes", "No"})
    );
  }
  else{
    answer = "Yes";
  }
  if(answer == "Yes") {
    for( auto& mod : _gameSelector_.getEntryList() ){
      _modManager_.removeMod(mod.title, 0);
    }
  }
  Switch::IO::p.useCrcCheck = true;
}

int ModBrowser::getPathDepth(const std::string& path_){
  int pathDepth = int((splitString(path_, "/")).size() );
  if(doesStringStartsWithSubstring(path_, "/")) pathDepth--;
  if(doesStringEndsWithSubstring(path_, "/")) pathDepth--;
  return pathDepth;
}

