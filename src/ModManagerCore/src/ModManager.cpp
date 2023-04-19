//
// Created by Nadrino on 06/09/2019.
//

#include "ModManager.h"
#include <Toolbox.h>
#include <ModBrowser.h>
#include "GlobalObjects.h"

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <utility>

ModManager::ModManager() {

  _internal_parameters_handler_ = false;
  reset();

}
ModManager::~ModManager(){

  reset();

}

void ModManager::initialize() {

  if(_parameters_handler_ptr_ == nullptr){
    _internal_parameters_handler_ = true;
    _parameters_handler_ptr_ = new ConfigHandler();
  }

}
void ModManager::reset(){

  _install_mods_base_folder_ = "/atmosphere/"; // should not be used
  _current_mods_folder_path_ = "";
  _use_cache_only_for_status_check_ = false;

  if(_internal_parameters_handler_ and _parameters_handler_ptr_ != nullptr) delete _parameters_handler_ptr_;
  _parameters_handler_ptr_ = nullptr;
  _internal_parameters_handler_ = false;

}

void ModManager::set_install_mods_base_folder(std::string install_mods_base_folder_){
  _install_mods_base_folder_ = std::move(install_mods_base_folder_);
}
void ModManager::set_use_cache_only_for_status_check(bool use_cache_only_for_status_check_){
  _use_cache_only_for_status_check_ = use_cache_only_for_status_check_;
}
void ModManager::set_ignored_file_list(std::vector<std::string>& ignored_file_list_){
  _ignored_file_list_ = ignored_file_list_;
}

std::string ModManager::get_install_mods_base_folder() {
  return _install_mods_base_folder_;
}
std::string & ModManager::getCurrentModFolderPath(){
  return _current_mods_folder_path_;
}
std::vector<std::string>& ModManager::get_ignored_file_list(){
  return _ignored_file_list_;
}

void ModManager::set_parameters_handler_ptr(ConfigHandler *parameters_handler_ptr_){
  _parameters_handler_ptr_ = parameters_handler_ptr_;
}
void ModManager::setCurrentModsFolder(const std::string &folder_path_) {
  _current_mods_folder_path_ = folder_path_;
//  _relative_file_path_list_cache_.clear();
  _mods_status_cache_.clear();
  _mods_status_cache_fraction_.clear();
  load_mods_status_cache_file();
}
void ModManager::load_mods_status_cache_file() {

  _mods_status_cache_.clear();
  _mods_status_cache_fraction_.clear();
  std::string cache_file_path = _current_mods_folder_path_ + "/mods_status_cache.txt";
  if(GenericToolbox::doesPathIsFile(cache_file_path)){

    auto lines = GenericToolbox::dumpFileAsVectorString(cache_file_path);
    for(int i_line = 0 ; i_line < int(lines.size()) ; i_line++){
      auto line_elements = GenericToolbox::splitString(lines[i_line], "=");
      if(line_elements.size() < 2) continue; // useless entry

      int index_mod_name = 0;
      if(line_elements.size() == 4){ // TO BE SUPPRESSED -> old test
        index_mod_name = 1;
      }
      _mods_status_cache_[line_elements[index_mod_name]] = line_elements[index_mod_name+1];
      if(line_elements.size() < 3) continue; // v < 1.5.0
      // v >= 1.5.0
      _mods_status_cache_fraction_[line_elements[index_mod_name]] = std::stod(line_elements[index_mod_name+2]);
    }

  }

}
void ModManager::save_mods_status_cache_file() {

  std::string cache_file_path = _current_mods_folder_path_ + "/mods_status_cache.txt";
  std::string data_string;

  for(auto const &mod_status : _mods_status_cache_){
    if(not mod_status.second.empty()){
      data_string += mod_status.first;
      data_string += "=" ;
      data_string += mod_status.second;
      data_string += "=" ;
      data_string += std::to_string(_mods_status_cache_fraction_[mod_status.first]) ;
      data_string += "\n";
    }
  }

  GenericToolbox::dumpStringInFile(cache_file_path, data_string);

}
void ModManager::reset_mod_cache_status(std::string mod_name_){
  _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = "";
  _mods_status_cache_fraction_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = -1;
}
void ModManager::resetAllModsCacheStatus(){

  GenericToolbox::deleteFile(_current_mods_folder_path_ + "/mods_status_cache.txt");
  load_mods_status_cache_file();

//  for(auto &mod_status_cache : _mods_status_cache_){
//    if(GenericToolbox::doesStringStartsWithSubstring(mod_status_cache.first, _parameters_handler_ptr_->get_selected_install_preset_name())){
//      _mods_status_cache_[mod_status_cache.first] = "";
//      _mods_status_cache_fraction_[mod_status_cache.first] = -1;
//    }
//  }
}

double ModManager::get_mod_status_fraction(std::string mod_name_){
  getModStatus(mod_name_);
  return _mods_status_cache_fraction_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_];
}
std::string ModManager::getModStatus(std::string mod_name_){

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE

  if(not _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_].empty())
    return _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_];

  if(_use_cache_only_for_status_check_)
    return "Not Checked";

  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  int same_files_count = 0;

  GenericToolbox::Switch::Terminal::printLeft("   Checking : Listing mod files...", GenericToolbox::ColorCodes::magentaBackground, true);
  consoleUpdate(nullptr);
  std::vector<std::string> relative_file_path_list;
//  if(_relative_file_path_list_cache_[mod_name_].empty()){
//    relative_file_path_list = GenericToolbox::getListFilesInSubfolders(absolute_mod_folder_path);
//    _relative_file_path_list_cache_[mod_name_] = relative_file_path_list;
//  } else{
//    relative_file_path_list = _relative_file_path_list_cache_[mod_name_];
//  }

  relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(absolute_mod_folder_path);

  int total_files_count = relative_file_path_list.size();
  Toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < total_files_count ; i_file++){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

    GenericToolbox::Switch::Terminal::displayProgressBar(
        i_file, total_files_count,
      "Checking : (" + std::to_string(i_file + 1) + "/" + std::to_string(total_files_count) + ") " +
      GenericToolbox::getFileNameFromFilePath(absolute_file_path)
      );

    if(GenericToolbox::Switch::IO::doFilesAreIdentical(
      _install_mods_base_folder_ + "/" + relative_file_path_list[i_file],
      absolute_file_path
    )) same_files_count++;
  }

  _mods_status_cache_fraction_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = double(same_files_count) / double(total_files_count);

  if(same_files_count == total_files_count) _mods_status_cache_[
                                              _parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = "ACTIVE";
  else if(same_files_count == 0) _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = "INACTIVE";
  else _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = "PARTIAL (" + std::to_string(same_files_count) + "/" + std::to_string(total_files_count) + ")";

  save_mods_status_cache_file();
  return _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_];

}
void ModManager::applyMod(const std::string& mod_name_, bool force_) {

  GenericToolbox::Switch::Terminal::printLeft("Applying : " + mod_name_ + "...", GenericToolbox::ColorCodes::greenBackground);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;
  GenericToolbox::Switch::Terminal::printLeft("   Getting files list...", GenericToolbox::ColorCodes::greenBackground, true);

//  if(_relative_file_path_list_cache_[mod_name_].empty()){
//    relative_file_path_list = GenericToolbox::getListFilesInSubfolders(absolute_mod_folder_path);
//    _relative_file_path_list_cache_[mod_name_] = relative_file_path_list;
//  } else {
//    relative_file_path_list = _relative_file_path_list_cache_[mod_name_];
//  }

  relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(absolute_mod_folder_path);

  // deleting ignored entries
  for(int i_mod = int(relative_file_path_list.size())-1 ; i_mod >= 0 ; i_mod--){
    if(GenericToolbox::doesElementIsInVector(relative_file_path_list[i_mod], _ignored_file_list_)){
      relative_file_path_list.erase(relative_file_path_list.begin() + i_mod);
    }
  }

  std::string replace_option;
  if(force_) replace_option = "Yes to all";
  bool is_conflict;
  std::stringstream ss_files_list;

  Toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(relative_file_path_list.size()) ; i_file++){

    if(relative_file_path_list[i_file][0] == '.'){
      // ignoring cached files
      continue;
    }
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

    std::string file_size = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(absolute_file_path)));

    GenericToolbox::Switch::Terminal::displayProgressBar(
        i_file, int(relative_file_path_list.size()),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(relative_file_path_list.size()) + ") " +
      GenericToolbox::getFileNameFromFilePath(relative_file_path_list[i_file]) + " (" + file_size + ")");

    std::string install_path = _install_mods_base_folder_ + "/" + relative_file_path_list[i_file];
    if(GenericToolbox::doesPathIsFile(install_path)) {
      is_conflict = true;
      if (replace_option == "Yes to all") {
        // remove log entry ? if log enabled
      }
      else if (replace_option == "No to all") {
        continue; // do nothing
      }
      else {
        replace_option = ask_to_replace(relative_file_path_list[i_file]);
        std::cout << ss_files_list.str();
      }
    }
    else {
      is_conflict = false;
    }
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      GenericToolbox::Switch::IO::copyFile(absolute_file_path, install_path);
    }
  }
  reset_mod_cache_status(mod_name_);

}
void ModManager::applyModList(const std::vector<std::string> &modNamesList_){

  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(modNamesList_.size());
  for(int i_mod = int(modNamesList_.size()) - 1 ; i_mod >= 0 ; i_mod--){
    std::string mod_path = _current_mods_folder_path_ + "/" + modNamesList_[i_mod];
    auto mod_files_list = GenericToolbox::getListOfFilesInSubFolders(mod_path);
    for(auto& mod_file : mod_files_list){
      if(GenericToolbox::doesElementIsInVector(mod_file, applied_files_listing)){
        mods_ignored_files_list[i_mod].emplace_back(mod_file);
      }
      else {
        applied_files_listing.emplace_back(mod_file);
      }
    }
  }

  // applying mods with ignored files
  for(int i_mod = 0 ; i_mod < int(modNamesList_.size()) ; i_mod++){
    _ignored_file_list_ = mods_ignored_files_list[i_mod];
    applyMod(modNamesList_[i_mod], true); // normally should work without force (tested) but just in case...
    _ignored_file_list_.clear();
  }

}
void ModManager::removeMod(std::string mod_name_){

  GenericToolbox::Switch::Terminal::printLeft("Disabling : " + mod_name_, GenericToolbox::ColorCodes::redBackground);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;

  relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(absolute_mod_folder_path);

  int i_file=0;
  Toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    i_file++;
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    absolute_file_path = GenericToolbox::removeRepeatedCharacters(absolute_file_path, "/");
    std::string file_size = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(absolute_file_path)));

    GenericToolbox::Switch::Terminal::displayProgressBar(
        i_file, relative_file_path_list.size(),
      GenericToolbox::getFileNameFromFilePath(relative_file_path) + " (" + file_size + ")"
      );

    std::string installed_file_path = _install_mods_base_folder_ + "/" + relative_file_path;
    installed_file_path = GenericToolbox::removeRepeatedCharacters(installed_file_path, "/");
    // Check if the installed mod belongs to the selected mod
    if( GenericToolbox::Switch::IO::doFilesAreIdentical( absolute_file_path, installed_file_path ) ){

      // Remove the mod file
      GenericToolbox::deleteFile(installed_file_path);

      // Delete the folder if no other files is present
      std::string empty_folder_path_candidate = GenericToolbox::getFolderPathFromFilePath(installed_file_path);
      while( GenericToolbox::isFolderEmpty( empty_folder_path_candidate ) ) {

        GenericToolbox::deleteEmptyDirectory(empty_folder_path_candidate);

        std::vector<std::string> sub_folder_list = GenericToolbox::splitString(empty_folder_path_candidate, "/");
        if(sub_folder_list.empty()) break; // virtually impossible -> would mean everything has been deleted on the sd
        // decrement folder depth
        empty_folder_path_candidate =
          "/" + GenericToolbox::joinVectorString(
            sub_folder_list,
            "/",
            0,
            int(sub_folder_list.size()) - 1
            );
      }
    }
  }
  reset_mod_cache_status(mod_name_);

}

void ModManager::display_mod_files_status(std::string mod_folder_path_){

  std::vector<std::string> file_path_list;
  GenericToolbox::Switch::Terminal::printLeft("Listing Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);

  file_path_list = GenericToolbox::getListOfFilesInSubFolders(mod_folder_path_);
  Selector sel;
  sel.setEntryList(file_path_list);
  GenericToolbox::Switch::Terminal::printLeft("Checking Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);
  Toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(file_path_list.size()) ; i_file++){

    GenericToolbox::Switch::Terminal::displayProgressBar(
        i_file, file_path_list.size(),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(file_path_list.size()) + ") " +
      GenericToolbox::getFileNameFromFilePath(file_path_list[i_file])
      );

    std::string installed_file_path = _install_mods_base_folder_ + "/" + file_path_list[i_file];
    if(GenericToolbox::Switch::IO::doFilesAreIdentical(
        mod_folder_path_+ "/" + file_path_list[i_file],
        installed_file_path
        )){
      sel.setTag(i_file, "-> Installed");
    } else if(GenericToolbox::doesPathIsFile(installed_file_path)){
      sel.setTag(i_file, "-> Not Same");
    } else {
      sel.setTag(i_file, "-> Not Installed");
    }
  }

  sel.setMaxItemsPerPage(GenericToolbox::Switch::Hardware::getTerminalHeight() - 9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      GenericToolbox::Switch::Terminal::printLeft(mod_folder_path_, GenericToolbox::ColorCodes::redBackground);
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      sel.print();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("Page (" + std::to_string(sel.getCursorPage() + 1) + "/" + std::to_string(
          sel.getNbPages()) + ")");
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" B : Go back", "");
      if(sel.getNbPages() > 1) GenericToolbox::Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
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

    sel.scanInputs(kDown, kHeld);

  }


}

std::string ModManager::ask_to_replace(std::string path_) {

  std::vector<std::string> options;
  options.emplace_back("Yes");
  options.emplace_back("Yes to all");
  options.emplace_back("No");
  options.emplace_back("No to all");
  return Selector::ask_question(path_ + " already exists. Replace ?", options);

}

std::map<std::string, std::string> & ModManager::getModsStatusCache() {
  return _mods_status_cache_;
}

bool ModManager::isUseCacheOnlyForStatusCheck() {
  return _use_cache_only_for_status_check_;
}

std::map<std::string, double> &ModManager::getModsStatusCacheFraction() {
  return _mods_status_cache_fraction_;
}

ConfigHandler *ModManager::getParametersHandlerPtr() {
  return _parameters_handler_ptr_;
}

