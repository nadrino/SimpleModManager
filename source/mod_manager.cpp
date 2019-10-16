//
// Created by Adrien Blanchet on 06/09/2019.
//

#include "mod_manager.h"
#include <toolbox.h>
#include <browser.h>

#include <iostream>
#include <sys/stat.h>
#include <switch.h>
#include <sstream>

mod_manager::mod_manager() {

  _files_list_cache_.clear();
  _custom_firmware_path_ = toolbox::get_cfw_config();

}
mod_manager::~mod_manager() {


}

void mod_manager::initialize() {



}

void mod_manager::set_log_file_path(std::string log_file_path_) {
  _log_file_path_ = log_file_path_;
}

std::string mod_manager::get_mod_status(std::string mod_folder_path_){

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE

  int same_files_count = 0;
  std::vector<std::string> files_list;
  if(_files_list_cache_[mod_folder_path_].empty()){
    files_list = toolbox::get_list_files_in_subfolders(mod_folder_path_);
    _files_list_cache_[mod_folder_path_] = files_list;
  } else{
    files_list = _files_list_cache_[mod_folder_path_];
  }

  int total_files_count = files_list.size();
  for(int i_file = 0 ; i_file < total_files_count ; i_file++){
    toolbox::display_loading(
        i_file, total_files_count,
        toolbox::get_filename_from_file_path(files_list[i_file]),
        toolbox::red_bg + "   "
        );
    if(toolbox::do_files_are_the_same(
        _custom_firmware_path_ + "/" + files_list[i_file],
        mod_folder_path_ + "/" + files_list[i_file]
    )) same_files_count++;
  }

  if(same_files_count == total_files_count) return "ACTIVE";
  if(same_files_count == 0) return "INACTIVE";

  return "PARTIAL (" + std::to_string(same_files_count) + "/" + std::to_string(total_files_count) + ")";

}
void mod_manager::apply_mod(std::string mod_folder_path_) {

  toolbox::print_left("Applying : " + toolbox::get_filename_from_file_path(mod_folder_path_) + "...", toolbox::red_bg);
  std::vector<std::string> file_path_list;
  if(_files_list_cache_[mod_folder_path_].empty()){
    file_path_list = toolbox::get_list_files_in_subfolders(mod_folder_path_);
    _files_list_cache_[mod_folder_path_] = file_path_list;
  } else{
    file_path_list = _files_list_cache_[mod_folder_path_];
  }
  std::string replace_option;
  bool is_conflict;
  std::stringstream ss_files_list;
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < int(file_path_list.size()) ; i_file++){

    toolbox::display_loading(
        i_file, int(file_path_list.size()),
        toolbox::get_filename_from_file_path(file_path_list[i_file]),
        toolbox::red_bg + "   ");

    if(toolbox::do_path_is_file(_custom_firmware_path_ + "/" + file_path_list[i_file])) {
      is_conflict = true;
      if (replace_option == "Yes to all") {
        // remove log entry ? if log enabled
      } else if (replace_option == "No to all") {
        continue; // do nothing
      } else {
        replace_option = ask_to_replace(file_path_list[i_file]);
        std::cout << ss_files_list.str();
      }
    } else {
      is_conflict = false;
    }
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      toolbox::copy_file(
          mod_folder_path_ + "/" + file_path_list[i_file],
          _custom_firmware_path_ + "/" + file_path_list[i_file]
          );
    }
  }

}
void mod_manager::remove_mod(std::string mod_folder_path_){

  std::stringstream title_ss;
  title_ss << "Disabling : " << toolbox::get_filename_from_file_path(mod_folder_path_) << "...";
  title_ss << toolbox::repeat_string(" ", toolbox::get_terminal_width() - int(title_ss.str().size()));
  std::cout << toolbox::red_bg << title_ss.str() << toolbox::reset_color; // should be end of line
  std::vector<std::string> file_path_list;
  if(_files_list_cache_[mod_folder_path_].empty()){
    file_path_list = toolbox::get_list_files_in_subfolders(mod_folder_path_);
    _files_list_cache_[mod_folder_path_] = file_path_list;
  } else{
    file_path_list = _files_list_cache_[mod_folder_path_];
  }
  int i_file=0;
  for(auto const &file_path : file_path_list){
    toolbox::display_loading(
        i_file, file_path_list.size(),
        toolbox::get_filename_from_file_path(file_path),
        toolbox::red_bg + "   "
        );
    i_file++;
    if(toolbox::do_path_is_file(_custom_firmware_path_ + "/" + file_path)) {
      if(toolbox::do_files_are_the_same(
          mod_folder_path_ + "/" + file_path,
          _custom_firmware_path_ + "/" + file_path)){
        toolbox::rm_file(_custom_firmware_path_ + "/" + file_path);
      }
      auto folder_path = toolbox::get_folder_path_from_file_path(file_path);
      while(toolbox::do_folder_is_empty(_custom_firmware_path_ + "/" + folder_path)){
        toolbox::rm_dir(_custom_firmware_path_ + "/" + folder_path);
        auto splitted_path = toolbox::split_string(folder_path, "/");
        if(splitted_path.empty()) break;
        folder_path = toolbox::join_vector_string(
            splitted_path,
            "/",
            0,
            int(splitted_path.size()) - 1
        );
      }
    }
  }

}
void mod_manager::display_mod_files_status(std::string mod_folder_path_){

  std::vector<std::string> file_path_list;
  if(_files_list_cache_[mod_folder_path_].empty()){
    file_path_list = toolbox::get_list_files_in_subfolders(mod_folder_path_);
    _files_list_cache_[mod_folder_path_] = file_path_list;
  } else{
    file_path_list = _files_list_cache_[mod_folder_path_];
  }
  selector sel;
  sel.set_selection_list(file_path_list);
  for(int i_file = 0 ; i_file < int(file_path_list.size()) ; i_file++){
    if(toolbox::do_files_are_the_same(
        mod_folder_path_+ "/" + file_path_list[i_file],
        _custom_firmware_path_ + "/" + file_path_list[i_file]
        )){
      sel.set_tag(i_file, "-> Installed");
    } else if(toolbox::do_path_is_file(_custom_firmware_path_ + "/" + file_path_list[i_file])){
      sel.set_tag(i_file, "-> Not Same");
    } else {
      sel.set_tag(i_file, "-> Not Installed");
    }
  }

  consoleClear();
  std::cout << mod_folder_path_ << std::endl;
  sel.print_selector();
  consoleUpdate(nullptr);

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    if (kDown & KEY_B) {
      break; // break in order to return to hbmenu
    } else if(kDown & KEY_DOWN){
      sel.increment_cursor_position();
    } else if(kDown & KEY_UP){
      sel.decrement_cursor_position();
    }

    if(kDown != 0){
      consoleClear();
      std::cout << mod_folder_path_ << std::endl;
      sel.print_selector();
      consoleUpdate(nullptr);
    }

  }


}

std::string mod_manager::ask_to_replace(std::string path_) {

  auto sel = selector();
  std::vector<std::string> options;
  options.emplace_back("Yes");
  options.emplace_back("Yes to all");
  options.emplace_back("No");
  options.emplace_back("No to all");
  sel.set_selection_list(options);

  consoleClear();
  std::cout << path_ << " already exists. Replace ?" << std::endl;
  sel.print_selector();

  while(appletMainLoop()){
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

    if(kDown & KEY_DOWN){
      sel.increment_cursor_position();
      consoleClear();
      std::cout << path_ << " already exists. Replace ?" << std::endl;
      sel.print_selector();
    } else if(kDown & KEY_UP){
      sel.decrement_cursor_position();
      consoleClear();
      std::cout << path_ << " already exists. Replace ?" << std::endl;
      sel.print_selector();
    } else if(kDown & KEY_A){
      consoleClear();
      return sel.get_selected_string();
    }
    consoleUpdate(nullptr);
  }

  return std::__cxx11::string();
}

