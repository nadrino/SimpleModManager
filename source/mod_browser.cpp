//
// Created by Adrien Blanchet on 03/09/2019.
//

#include <mod_browser.h>
#include <toolbox.h>

#include <iostream>
#include <switch.h>
#include <algorithm>
#include <iomanip>


mod_browser::mod_browser(){

  reset();

}
mod_browser::~mod_browser()= default;

void mod_browser::initialize(){

  _selector_.initialize();

  _parameters_handler_.initialize();
  set_base_folder(_parameters_handler_.get_parameter("stored-mods-base-folder"));

  _mod_manager_.initialize();
  _mod_manager_.set_install_mods_base_folder(
    _parameters_handler_.get_parameter("install-mods-base-folder")
    );

  change_directory(_base_folder_);

}
void mod_browser::reset(){

  _max_relative_depth_ = -1;
  _base_folder_ = "/";
  _current_directory_ = _base_folder_;
  _only_show_folders_ = false;

  _selector_.reset();
  _parameters_handler_.reset();

}

void mod_browser::set_base_folder(std::string base_folder_){
  _base_folder_ = base_folder_;
}
void mod_browser::set_max_relative_depth(int max_relative_depth_){
  _max_relative_depth_ = max_relative_depth_;
}
void mod_browser::set_only_show_folders(bool only_show_folders_){
  _only_show_folders_ = only_show_folders_;
}

int mod_browser::get_current_relative_depth(){
  return _current_relative_depth_;
}
int mod_browser::get_max_relative_depth(){
  return _max_relative_depth_;
}
std::string mod_browser::get_current_directory(){
  return _current_directory_;
}
std::string mod_browser::get_base_folder(){
  return _base_folder_;
}
parameters_handler &mod_browser::get_parameters_handler(){
  return _parameters_handler_;
}

void mod_browser::scan_inputs(u64 kDown, u64 kHeld){

  _selector_.scan_inputs(kDown, kHeld);

  if(kDown == 0 and kHeld == 0) return;

  if(get_current_relative_depth() == get_max_relative_depth()){
    if(kDown & KEY_A){ // select folder / apply mod
      // make mod action (ACTIVATE OR DEACTIVATE)
      _mod_manager_.apply_mod(_selector_.get_selected_string());

      toolbox::print_left("Checking...", toolbox::magenta_bg, true);
      _selector_.set_tag(
        _selector_.get_selected_entry(),
        _mod_manager_.get_mod_status(_selector_.get_selected_string())
      );

    }
    else if(kDown & KEY_X){ // disable mod
      _mod_manager_.remove_mod(_selector_.get_selected_string());
      _selector_.set_tag(
        _selector_.get_selected_entry(),
        _mod_manager_.get_mod_status(_selector_.get_selected_string())
      );
    }
    else if(kDown & KEY_ZL){ // recheck all mods
      auto answer = toolbox::ask_question("Do you which to recheck all mods ?",
        std::vector<std::string>({"Yes","No"}));
      if(answer == "Yes"){
        _mod_manager_.reset_all_mods_cache_status();
        check_mods_status();
      }
    }
    else if(kDown & KEY_Y){ // mod detailed infos
      _mod_manager_.display_mod_files_status(get_current_directory() + "/" + _selector_.get_selected_string());
    }
    else if(kDown & KEY_ZR){ // disable all mods
      remove_all_mods();
      check_mods_status();
    }
    else if(kDown & KEY_MINUS){ // Enter the mods preset menu a mods preset
      _mods_preseter_.select_mod_preset();
    }
    else if(kDown & KEY_PLUS){ // Apply a mods preset
      std::string answer = toolbox::ask_question(
        "Do you want to apply " + _mods_preseter_.get_selected_mod_preset() + " ?",
        std::vector<std::string>({"Yes","No"})
        );
      if(answer == "Yes"){
        remove_all_mods(true);
        std::vector<std::string> mods_list = _mods_preseter_.get_mods_list(
          _mods_preseter_.get_selected_mod_preset()
        );
        for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){
          _mod_manager_.apply_mod(mods_list[i_mod],true);
        }
        check_mods_status();
      }
    }
    else if(kDown & KEY_L){
      _mods_preseter_.select_previous_mod_preset();
    }
    else if(kDown & KEY_R){
      _mods_preseter_.select_next_mod_preset();
    }
  }
  else {
    if(kDown & KEY_A){ // select folder / apply mod
      go_to_selected_directory();
      if(get_current_relative_depth() == get_max_relative_depth()){
        _mod_manager_.set_current_mods_folder(_current_directory_);
        check_mods_status();
        _mods_preseter_.read_parameter_file(_current_directory_);
      }
    }
    else if(kDown & KEY_Y){ // switch between config preset
      if(get_current_relative_depth() == 0){
        _parameters_handler_.increment_selected_preset_id();
        _mod_manager_.set_install_mods_base_folder(
          _parameters_handler_.get_parameter("install-mods-base-folder")
          );
      }
    }
  }

  if(kDown & KEY_B){ // back
    if(not go_back()){
      std::cout << "Can't go back." << std::endl;
    }
  }

  print_menu();

}
void mod_browser::print_menu(){
  consoleClear();
  toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());

  // ls
  toolbox::print_left("Current Folder : " + _current_directory_, toolbox::red_bg);
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  _selector_.print_selector();
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());

  std::cout << "  Page (" << _selector_.get_current_page() + 1 << "/" << _selector_.get_nb_pages() << ")" << std::endl;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  toolbox::print_left("Mod preset : " + _mods_preseter_.get_selected_mod_preset());
  toolbox::print_left("Configuration preset : " + _parameters_handler_.get_selected_preset_name());
  toolbox::print_left("install-mods-base-folder = " + _mod_manager_.get_install_mods_base_folder());
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  if(get_current_relative_depth() == get_max_relative_depth()){
    toolbox::print_left_right(" A : Apply mod", "X : Disable mod ");
    toolbox::print_left_right(" ZL : Rescan all mods", "ZR : Disable all mods ");
    toolbox::print_left_right(" - : Select mod preset", "+ : Apply mod preset ");
  }
  else{
    toolbox::print_left_right(" A : Select folder", "Y : Change config preset ");
  }
  if(get_current_relative_depth() > 0) toolbox::print_left(" B : Go back");
  else toolbox::print_left(" B : Quit");
  consoleUpdate(nullptr);

}
void mod_browser::check_mods_status(){
  if(get_current_relative_depth() != get_max_relative_depth()) return;
  _selector_.reset_tags_list();
  auto mods_list = _selector_.get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){
    hidScanInput();
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    if(kDown & KEY_B) break;

    _selector_.set_tag(i_mod, "Checking...");
    print_menu();
    std::stringstream ss;
    ss << "Checking ("<< i_mod+1 << "/" << mods_list.size() << ") : " << mods_list[i_mod] << "...";
    toolbox::print_left(ss.str(), toolbox::magenta_bg);
    consoleUpdate(nullptr);
    _selector_.set_tag(i_mod, _mod_manager_.get_mod_status(mods_list[i_mod]));
  }
}
bool mod_browser::change_directory(std::string new_directory_){

  if(not toolbox::do_path_is_folder(new_directory_)) return false;
  auto new_path_relative_depth = get_relative_path_depth(new_directory_);
  if(new_path_relative_depth != -1 and new_path_relative_depth > _max_relative_depth_) return false;

  _current_directory_ = new_directory_;
  _current_relative_depth_ = new_path_relative_depth;
  std::vector<std::string> selection_list;
  if(not _only_show_folders_)selection_list = toolbox::get_list_of_entries_in_folder(_current_directory_);
  else selection_list = toolbox::get_list_of_subfolders_in_folder(_current_directory_);
  std::sort(selection_list.begin(), selection_list.end());
  _selector_.set_selection_list(selection_list);
  _selector_.reset_cursor_position();
  _selector_.reset_page();
  return true;

}
bool mod_browser::go_to_selected_directory(){
  std::string new_path = _current_directory_ + "/" + _selector_.get_selected_string();
  new_path = toolbox::remove_extra_doubled_characters(new_path, "/");
  return change_directory(new_path);
}
bool mod_browser::go_back(){

  if(get_relative_path_depth(_current_directory_) <= 0 or _current_directory_ == "/") return false; // already at maximum root

  auto folder_elements = toolbox::split_string(_current_directory_, "/");
  auto new_path = "/" + toolbox::join_vector_string(folder_elements, "/", 0, folder_elements.size()-1);
  new_path = toolbox::remove_extra_doubled_characters(new_path, "/");
  if(not toolbox::do_path_is_folder(new_path)){
    std::cerr << "Can't go back, \"" << new_path << "\" is not a folder" << std::endl;
    return false;
  }
  return change_directory(new_path);

}
int mod_browser::get_relative_path_depth(std::string& path_){
  int path_depth = get_path_depth(path_);
  int base_depth = get_path_depth(_base_folder_);
  int relative_path_depth = path_depth - base_depth;
  return relative_path_depth;
}
int mod_browser::get_path_depth(std::string& path_){
  int path_depth = (toolbox::split_string(path_, "/")).size() ;
  if(toolbox::do_string_starts_with_substring(path_, "/")) path_depth--;
  if(toolbox::do_string_ends_with_substring(path_, "/")) path_depth--;
  return path_depth;
}

void mod_browser::remove_all_mods(bool force_){
  std::string answer;
  bool CRC_check_state = toolbox::get_CRC_check_is_enabled();
  toolbox::set_CRC_check_is_enabled(false);
  if(not force_){
    answer = toolbox::ask_question(
      "Do you want to disable all mods ?",
      std::vector<std::string>({"Yes","No"})
    );
  } else{
    answer = "Yes";
  }
  if(answer == "Yes") {
    for(int i_mod = 0 ; i_mod < int(_selector_.get_selection_list().size()) ; i_mod++){
      _mod_manager_.remove_mod(_selector_.get_selection_list()[i_mod]);
    }
  }
  toolbox::set_CRC_check_is_enabled(CRC_check_state);
}

