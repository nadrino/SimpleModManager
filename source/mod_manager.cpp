//
// Created by Adrien Blanchet on 06/09/2019.
//

#include "mod_manager.h"
#include <toolbox.h>
#include <mod_browser.h>

#include <iostream>
#include <sys/stat.h>
#include <switch.h>
#include <sstream>
#include <fstream>

mod_manager::mod_manager() {

  _relative_file_path_list_cache_.clear();
  _install_mods_base_folder_ = "/atmosphere/"; // should not be used
  _current_mods_folder_path_ = "";
  _use_cache_only_for_status_check_ = false;

}
mod_manager::~mod_manager() = default;

void mod_manager::initialize() {



}

void mod_manager::set_install_mods_base_folder(std::string install_mods_base_folder_){
  _install_mods_base_folder_ = install_mods_base_folder_;
}
void mod_manager::set_use_cache_only_for_status_check(bool use_cache_only_for_status_check_){
  _use_cache_only_for_status_check_ = use_cache_only_for_status_check_;
}

std::string mod_manager::get_install_mods_base_folder() {
  return _install_mods_base_folder_;
}

void mod_manager::set_current_mods_folder(std::string folder_path_) {
  _current_mods_folder_path_ = folder_path_;
  _relative_file_path_list_cache_.clear();
  _mods_status_cache_.clear();
  _mods_status_cache_fraction_.clear();
  load_mods_status_cache_file();
}
void mod_manager::load_mods_status_cache_file() {

  _mods_status_cache_.clear();
  _mods_status_cache_fraction_.clear();
  std::string cache_file_path = _current_mods_folder_path_ + "/mods_status_cache";
  if(toolbox::do_path_is_file(cache_file_path)){

    std::ifstream cache_file;
    cache_file.open (cache_file_path.c_str());
    std::string line;
    while( std::getline(cache_file, line) ){
      auto line_elements = toolbox::split_string(line, "=");
      if(line_elements.size() < 2) continue;
      _mods_status_cache_[line_elements[0]] = line_elements[1];
      if(line_elements.size() < 3) continue;
      _mods_status_cache_fraction_[line_elements[0]] = std::stod(line_elements[2]);
    }
    cache_file.close();

  }

}
void mod_manager::save_mods_status_cache_file() {

  std::string cache_file_path = _current_mods_folder_path_ + "/mods_status_cache";
  std::ofstream cache_file;
  cache_file.open (cache_file_path.c_str());

  for(auto const &mod_status : _mods_status_cache_){
    if(not mod_status.second.empty()){
      cache_file << mod_status.first << "=" << mod_status.second << "=" << _mods_status_cache_fraction_[mod_status.first] << std::endl;
    }
  }

  cache_file.close();

}
void mod_manager::reset_mod_cache_status(std::string mod_name_){
  _mods_status_cache_[mod_name_] = "";
  _mods_status_cache_fraction_[mod_name_] = -1;
}
void mod_manager::reset_all_mods_cache_status(){
  for(auto &mod_status_cache : _mods_status_cache_){
    _mods_status_cache_[mod_status_cache.first] = "";
    _mods_status_cache_fraction_[mod_status_cache.first] = -1;
  }
}

double mod_manager::get_mod_status_fraction(std::string mod_name_){
  get_mod_status(mod_name_);
  return _mods_status_cache_fraction_[mod_name_];
}
std::string mod_manager::get_mod_status(std::string mod_name_){

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE

  if(not _mods_status_cache_[mod_name_].empty())
    return _mods_status_cache_[mod_name_];

  if(_use_cache_only_for_status_check_)
    return "Not Checked";

  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  int same_files_count = 0;

  toolbox::print_left("   Checking : Listing mod files...", toolbox::magenta_bg, true);
  consoleUpdate(nullptr);
  std::vector<std::string> relative_file_path_list;
  if(_relative_file_path_list_cache_[mod_name_].empty()){
    relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);
    _relative_file_path_list_cache_[mod_name_] = relative_file_path_list;
  } else{
    relative_file_path_list = _relative_file_path_list_cache_[mod_name_];
  }

  int total_files_count = relative_file_path_list.size();
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < total_files_count ; i_file++){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

    toolbox::display_loading(
      i_file, total_files_count,
      "Checking : (" + std::to_string(i_file + 1) + "/" + std::to_string(total_files_count) + ") " +
      toolbox::get_filename_from_file_path(absolute_file_path),
      "   ",
      toolbox::magenta_bg
      );

    if(toolbox::do_files_are_the_same(
      _install_mods_base_folder_ + "/" + relative_file_path_list[i_file],
      absolute_file_path
    )) same_files_count++;
  }

  _mods_status_cache_fraction_[mod_name_] = double(same_files_count)/double(total_files_count);

  if(same_files_count == total_files_count) _mods_status_cache_[mod_name_] = "ACTIVE";
  else if(same_files_count == 0) _mods_status_cache_[mod_name_] = "INACTIVE";
  else _mods_status_cache_[mod_name_] = "PARTIAL (" + std::to_string(same_files_count) + "/" + std::to_string(total_files_count) + ")";

  save_mods_status_cache_file();
  return _mods_status_cache_[mod_name_];

}
void mod_manager::apply_mod(std::string mod_name_, bool force_) {

  toolbox::print_left("Applying : " + mod_name_ + "...", toolbox::green_bg);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;
  toolbox::print_left("   Getting files list...", toolbox::green_bg, true);
  if(_relative_file_path_list_cache_[mod_name_].empty()){
    relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);
    _relative_file_path_list_cache_[mod_name_] = relative_file_path_list;
  } else {
    relative_file_path_list = _relative_file_path_list_cache_[mod_name_];
  }

  std::string replace_option;
  if(force_) replace_option = "Yes to all";
  bool is_conflict;
  std::stringstream ss_files_list;
  toolbox::reset_last_displayed_value();

  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(relative_file_path_list.size()) ; i_file++){

    if(relative_file_path_list[i_file][0] == '.'){
      // ignoring cached files
      continue;
    }
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];
    std::string file_size = std::to_string(toolbox::get_file_size(absolute_file_path)/1000) + " kB";

    toolbox::display_loading(
      i_file, int(relative_file_path_list.size()),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(relative_file_path_list.size()) + ") " +
      toolbox::get_filename_from_file_path(relative_file_path_list[i_file]) + " (" + file_size + ")",
      "   ",
      toolbox::green_bg);

    std::string install_path = _install_mods_base_folder_ + "/" + relative_file_path_list[i_file];
    if(toolbox::do_path_is_file(install_path)) {
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
      toolbox::copy_file( absolute_file_path, install_path );
    }
  }
  reset_mod_cache_status(mod_name_);

}
void mod_manager::remove_mod(std::string mod_name_){

  toolbox::print_left("Disabling : " + mod_name_, toolbox::red_bg);
  std::string absolute_mod_folder_path = _current_mods_folder_path_ + "/" + mod_name_;

  std::vector<std::string> relative_file_path_list;
  if(_relative_file_path_list_cache_[mod_name_].empty()){
    relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);
    _relative_file_path_list_cache_[mod_name_] = relative_file_path_list;
  } else{
    relative_file_path_list = _relative_file_path_list_cache_[mod_name_];
  }

  int i_file=0;
  toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    i_file++;
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    std::string file_size = std::to_string(toolbox::get_file_size(absolute_file_path)/1000) + " kB";

    toolbox::display_loading(
      i_file, relative_file_path_list.size(),
      toolbox::get_filename_from_file_path(relative_file_path) + " (" + file_size + ")",
      "   ",
      toolbox::red_bg
      );

    std::string installed_file_path = _install_mods_base_folder_ + "/" + relative_file_path;
    // Check if the installed mod belongs to the selected mod
    if( toolbox::do_files_are_the_same( absolute_file_path, installed_file_path ) ){

      // Remove the mod file
      toolbox::rm_file( installed_file_path );

      // Delete the folder if no other files is present
      std::string empty_folder_path_candidate = toolbox::get_folder_path_from_file_path(installed_file_path);
      while( toolbox::do_folder_is_empty( empty_folder_path_candidate ) ){
        toolbox::rm_dir( empty_folder_path_candidate );
        std::vector<std::string> sub_folder_list = toolbox::split_string(empty_folder_path_candidate, "/");
        if(sub_folder_list.empty()) break; // virtually impossible -> would mean everything has been deleted on the sd
        // decrement folder depth
        empty_folder_path_candidate =
          toolbox::join_vector_string(
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
  if(_relative_file_path_list_cache_[mod_folder_path_].empty()){
    file_path_list = toolbox::get_list_files_in_subfolders(mod_folder_path_);
    _relative_file_path_list_cache_[mod_folder_path_] = file_path_list;
  } else{
    file_path_list = _relative_file_path_list_cache_[mod_folder_path_];
  }
  selector sel;
  sel.set_selection_list(file_path_list);
  toolbox::print_left("Checking Files...", toolbox::red_bg);
  consoleUpdate(nullptr);
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(file_path_list.size()) ; i_file++){

    toolbox::display_loading(
      i_file, file_path_list.size(),
      "(" + std::to_string(i_file + 1) + "/" + std::to_string(file_path_list.size()) + ") " +
      toolbox::get_filename_from_file_path(file_path_list[i_file]),
      "   ",
      toolbox::red_bg
      );

    std::string installed_file_path = _install_mods_base_folder_ + "/" + file_path_list[i_file];
    if(toolbox::do_files_are_the_same(
        mod_folder_path_+ "/" + file_path_list[i_file],
        installed_file_path
        )){
      sel.set_tag(i_file, "-> Installed");
    } else if(toolbox::do_path_is_file(installed_file_path)){
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
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      sel.print_selector();
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      toolbox::print_left("Page (" + std::to_string(sel.get_current_page()+1) + "/" + std::to_string(sel.get_nb_pages()) + ")");
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      toolbox::print_left_right(" B : Go back", "");
      if(sel.get_nb_pages() > 1) toolbox::print_left_right(" L : Previous Page", "R : Next Page ");
      consoleUpdate(nullptr);
    }

    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if (kDown & KEY_B) {
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

