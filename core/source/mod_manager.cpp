//
// Created by Nadrino on 06/09/2019.
//

#include "mod_manager.h"
#include <toolbox.h>
#include <mod_browser.h>
#include "GlobalObjects.h"

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <utility>

mod_manager::mod_manager() {

  _internal_parameters_handler_ = false;
  reset();

}
mod_manager::~mod_manager(){

  reset();

}

void mod_manager::initialize() {

  if(_parameters_handler_ptr_ == nullptr){
    _internal_parameters_handler_ = true;
    _parameters_handler_ptr_ = new parameters_handler();
  }

}
void mod_manager::reset(){

  _install_mods_base_folder_ = "/atmosphere/"; // should not be used
  _current_mods_folder_path_ = "";
  _use_cache_only_for_status_check_ = false;

  if(_internal_parameters_handler_ and _parameters_handler_ptr_ != nullptr) delete _parameters_handler_ptr_;
  _parameters_handler_ptr_ = nullptr;
  _internal_parameters_handler_ = false;

}

void mod_manager::set_install_mods_base_folder(std::string install_mods_base_folder_){
  _install_mods_base_folder_ = std::move(install_mods_base_folder_);
}
void mod_manager::set_use_cache_only_for_status_check(bool use_cache_only_for_status_check_){
  _use_cache_only_for_status_check_ = use_cache_only_for_status_check_;
}
void mod_manager::set_ignored_file_list(std::vector<std::string>& ignored_file_list_){
  _ignored_file_list_ = ignored_file_list_;
}

std::string mod_manager::get_install_mods_base_folder() {
  return _install_mods_base_folder_;
}
std::string & mod_manager::get_current_mods_folder_path(){
  return _current_mods_folder_path_;
}
std::vector<std::string>& mod_manager::get_ignored_file_list(){
  return _ignored_file_list_;
}

void mod_manager::set_parameters_handler_ptr(parameters_handler *parameters_handler_ptr_){
  _parameters_handler_ptr_ = parameters_handler_ptr_;
}
void mod_manager::set_current_mods_folder(std::string folder_path_) {
  _current_mods_folder_path_ = folder_path_;
//  _relative_file_path_list_cache_.clear();
  _mods_status_cache_.clear();
  _mods_status_cache_fraction_.clear();
  load_mods_status_cache_file();
}
void mod_manager::load_mods_status_cache_file() {

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
void mod_manager::save_mods_status_cache_file() {

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

  GenericToolbox::dumpStringInFile(data_string, cache_file_path);

}
void mod_manager::reset_mod_cache_status(std::string mod_name_){
  _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = "";
  _mods_status_cache_fraction_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_] = -1;
}
void mod_manager::reset_all_mods_cache_status(){

  GenericToolbox::deleteFile(_current_mods_folder_path_ + "/mods_status_cache.txt");
  load_mods_status_cache_file();

//  for(auto &mod_status_cache : _mods_status_cache_){
//    if(GenericToolbox::doesStringStartsWithSubstring(mod_status_cache.first, _parameters_handler_ptr_->get_selected_install_preset_name())){
//      _mods_status_cache_[mod_status_cache.first] = "";
//      _mods_status_cache_fraction_[mod_status_cache.first] = -1;
//    }
//  }
}

double mod_manager::get_mod_status_fraction(std::string mod_name_){
  get_mod_status(mod_name_);
  return _mods_status_cache_fraction_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_];
}
std::string mod_manager::get_mod_status(std::string mod_name_){

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE

  if(not _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_].empty())
    return _mods_status_cache_[_parameters_handler_ptr_->get_current_config_preset_name() + ": " + mod_name_];

  if(_use_cache_only_for_status_check_)
    return "Not Checked";

  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  int same_files_count = 0;

  toolbox::print_left("   Checking : Listing mod files...", toolbox::magenta_bg, true);
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
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < total_files_count ; i_file++){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

    toolbox::display_loading(
      i_file, total_files_count,
      "Checking : (" + std::to_string(i_file + 1) + "/" + std::to_string(total_files_count) + ") " +
      GenericToolbox::getFileNameFromFilePath(absolute_file_path),
      "   ",
      toolbox::magenta_bg
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
void mod_manager::apply_mod(std::string mod_name_, bool force_) {

  toolbox::print_left("Applying : " + mod_name_ + "...", toolbox::green_bg);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;
  toolbox::print_left("   Getting files list...", toolbox::green_bg, true);

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

  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(relative_file_path_list.size()) ; i_file++){

    if(relative_file_path_list[i_file][0] == '.'){
      // ignoring cached files
      continue;
    }
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    toolbox::display_loading(
      i_file, int(relative_file_path_list.size()),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(relative_file_path_list.size()) + ") " +
      GenericToolbox::getFileNameFromFilePath(relative_file_path_list[i_file]) + " (" + file_size + ")",
      "   ",
      toolbox::green_bg);

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
void mod_manager::apply_mod_list(std::vector<std::string> &mod_names_list_){

  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(mod_names_list_.size());
  for(int i_mod = int(mod_names_list_.size())-1 ; i_mod >= 0 ; i_mod--){
    std::string mod_path = _current_mods_folder_path_ + "/" + mod_names_list_[i_mod];
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
  for(int i_mod = 0 ; i_mod < int(mod_names_list_.size()) ; i_mod++){
    _ignored_file_list_ = mods_ignored_files_list[i_mod];
    apply_mod(mod_names_list_[i_mod], true); // normally should work without force (tested) but just in case...
    _ignored_file_list_.clear();
  }

}
void mod_manager::remove_mod(std::string mod_name_){

  toolbox::print_left("Disabling : " + mod_name_, toolbox::red_bg);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;

  relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(absolute_mod_folder_path);

  int i_file=0;
  toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    i_file++;
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    absolute_file_path = GenericToolbox::removeRepeatedCharacters(absolute_file_path, "/");
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    toolbox::display_loading(
      i_file, relative_file_path_list.size(),
      GenericToolbox::getFileNameFromFilePath(relative_file_path) + " (" + file_size + ")",
      "   ",
      toolbox::red_bg
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

void mod_manager::display_mod_files_status(std::string mod_folder_path_){

  std::vector<std::string> file_path_list;
  toolbox::print_left("Listing Files...", toolbox::red_bg);
  consoleUpdate(nullptr);

  file_path_list = GenericToolbox::getListOfFilesInSubFolders(mod_folder_path_);
  selector sel;
  sel.set_selection_list(file_path_list);
  toolbox::print_left("Checking Files...", toolbox::red_bg);
  consoleUpdate(nullptr);
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(file_path_list.size()) ; i_file++){

    toolbox::display_loading(
      i_file, file_path_list.size(),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(file_path_list.size()) + ") " +
      GenericToolbox::getFileNameFromFilePath(file_path_list[i_file]),
      "   ",
      toolbox::red_bg
      );

    std::string installed_file_path = _install_mods_base_folder_ + "/" + file_path_list[i_file];
    if(GenericToolbox::Switch::IO::doFilesAreIdentical(
        mod_folder_path_+ "/" + file_path_list[i_file],
        installed_file_path
        )){
      sel.set_tag(i_file, "-> Installed");
    } else if(GenericToolbox::doesPathIsFile(installed_file_path)){
      sel.set_tag(i_file, "-> Not Same");
    } else {
      sel.set_tag(i_file, "-> Not Installed");
    }
  }

  sel.set_max_items_per_page(toolbox::get_terminal_height()-9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      toolbox::print_left(mod_folder_path_, toolbox::red_bg);
      std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
      sel.print_selector();
      std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
      toolbox::print_left("Page (" + std::to_string(sel.get_current_page()+1) + "/" + std::to_string(sel.get_nb_pages()) + ")");
      std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
      toolbox::print_left_right(" B : Go back", "");
      if(sel.get_nb_pages() > 1) toolbox::print_left_right(" <- : Previous Page", "-> : Next Page ");
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

std::string mod_manager::ask_to_replace(std::string path_) {

  std::vector<std::string> options;
  options.emplace_back("Yes");
  options.emplace_back("Yes to all");
  options.emplace_back("No");
  options.emplace_back("No to all");
  return toolbox::ask_question(path_ + " already exists. Replace ?", options);

}

std::map<std::string, std::string> & mod_manager::get_mods_status_cache() {
  return _mods_status_cache_;
}

bool mod_manager::isUseCacheOnlyForStatusCheck() {
  return _use_cache_only_for_status_check_;
}

std::map<std::string, double> &mod_manager::getModsStatusCacheFraction() {
  return _mods_status_cache_fraction_;
}

parameters_handler *mod_manager::getParametersHandlerPtr() {
  return _parameters_handler_ptr_;
}

