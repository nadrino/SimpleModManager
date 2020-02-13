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

  _max_depth_ = -1;
  _base_folder_ = "/";
  _current_directory_ = _base_folder_;
  _selected_entry_name_ = "";
  _only_show_folders_ = false;

  _selector_.reset();
  _parameters_handler_.reset();

}

void mod_browser::set_base_folder(std::string base_folder_){
  _base_folder_ = base_folder_;
}
void mod_browser::set_max_depth(int max_depth_){
  _max_depth_ = max_depth_;
}
void mod_browser::set_only_show_folders(bool only_show_folders_){
  _only_show_folders_ = only_show_folders_;
}

int mod_browser::get_current_depth(){
  return _current_depth_;
}
int mod_browser::get_max_depth(){
  return _max_depth_;
}
std::string mod_browser::get_current_directory(){
  return _current_directory_;
}
std::string mod_browser::get_selected_entry_name() {
  return _selected_entry_name_;
}
selector& mod_browser::get_selector(){
  return _selector_;
}
mod_manager& mod_browser::get_mod_manager(){
  return _mod_manager_;
}

void mod_browser::scan_inputs(u64 kDown, u64 kHeld){

  _selector_.scan_inputs(kDown, kHeld);

  if(kDown == 0 and kHeld == 0) return;

  if(get_current_depth() == get_max_depth()){
    if(kDown & KEY_A){ // select folder / apply mod
      // make mod action (ACTIVATE OR DEACTIVATE)
      _mod_manager_.apply_mod(get_current_directory() + "/" + _selector_.get_selected_string());
      toolbox::print_left("Checking installed mod...", toolbox::red_bg);
        _selector_.set_tag(
        _selector_.get_selected_entry(),
        _mod_manager_.get_mod_status(get_current_directory() + "/" + _selector_.get_selected_string())
      );
    } else if(kDown & KEY_X){ // disable mod
      _mod_manager_.remove_mod(get_current_directory() + "/" + _selector_.get_selected_string());
      _selector_.set_tag(
        _selector_.get_selected_entry(),
        _mod_manager_.get_mod_status(get_current_directory() + "/" + _selector_.get_selected_string())
      );
    }  else if(kDown & KEY_ZL){ // mod detailed infos
      _mod_manager_.display_mod_files_status(get_current_directory() + "/" + _selector_.get_selected_string());
    } else if(kDown & KEY_ZR){ // disable all mods
      std::vector<std::string> options;
      options.emplace_back("Yes");
      options.emplace_back("No");
      std::string answer = toolbox::ask_question("Do you want to disable all mods ?", options);
      if(answer == "Yes") {
        for(int i_mod = 0 ; i_mod < int(_selector_.get_selection_list().size()) ; i_mod++){
          _mod_manager_.remove_mod(get_current_directory() + "/" + _selector_.get_selection_list()[i_mod]);
        }
        check_mods_status();
      }
    } else if(kDown & KEY_LEFT){ // previous between mods presets
      if(get_current_depth() == get_max_depth()){

      }
    } else if(kDown & KEY_RIGHT){ // next between mods presets

    } else if(kDown & KEY_MINUS){ // record a mods preset

    } else if(kDown & KEY_PLUS){ // apply a mods preset

    }
  } else {
    if(kDown & KEY_A){ // select folder / apply mod
      go_to_selected_directory();
      check_mods_status();
    } else if(kDown & KEY_Y){ // switch between config preset
      if(get_current_depth() == 0){
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
  std::cout << toolbox::red_bg << std::setw(toolbox::get_terminal_width()) << std::left;
  std::cout << "Current Folder : " + _current_directory_ << toolbox::reset_color;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  _selector_.print_selector();
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());

  std::cout << "Page (" << _selector_.get_current_page() + 1 << "/" << _selector_.get_nb_pages() << ")" << std::endl;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
//  toolbox::print_left("Mods preset : " + __current_mod_preset__);
  toolbox::print_left("Configuration preset : " + _parameters_handler_.get_selected_preset_name());
  toolbox::print_left("install-mods-base-folder = " + _mod_manager_.get_install_mods_base_folder());
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  if(get_current_depth() == get_max_depth()){
    toolbox::print_left_right(" A : Apply mod", "X : Disable mod ");
    toolbox::print_left_right(" ZL : Mod status", "ZR : Disable all mods ");
  } else{
    toolbox::print_left_right(" A : Select folder", "Y : Change config preset ");
  }
  toolbox::print_left_right(" B : Go back", "+/- : Quit ");
  if(_selector_.get_nb_pages() > 1) toolbox::print_left_right(" L : Previous Page", "R : Next Page ");
  consoleUpdate(nullptr);

}
void mod_browser::check_mods_status(){
  if(get_current_depth() != get_max_depth()) return;
  _selector_.reset_tags_list();
  auto mods_list = _selector_.get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){
    _selector_.set_tag(i_mod, "Checking...");
    print_menu();
    std::stringstream ss;
    ss << "Checking ("<< i_mod+1 << "/" << mods_list.size() << ") : " << mods_list[i_mod] << "...";
    auto spaces = toolbox::repeat_string(" ",toolbox::get_terminal_width() - ss.str().size());
    std::cout << toolbox::red_bg << ss.str() << spaces << toolbox::reset_color;
    consoleUpdate(nullptr);
    std::string mod_path = get_current_directory() + "/" + mods_list[i_mod];
    _selector_.set_tag(i_mod, _mod_manager_.get_mod_status(mod_path));
  }
}
bool mod_browser::change_directory(std::string new_directory_){

  if(not toolbox::do_path_is_folder(new_directory_)) return false;
  auto new_path_depth = get_path_depth(new_directory_);
  if(new_path_depth != -1 and new_path_depth > _max_depth_) return false;

  _current_directory_ = new_directory_;
  _current_depth_ = new_path_depth;
  std::vector<std::string> selection_list;
  if(not _only_show_folders_)selection_list = toolbox::get_list_of_entries_in_folder(_current_directory_);
  else selection_list = toolbox::get_list_of_folders_in_folder(_current_directory_);
  std::sort(selection_list.begin(), selection_list.end());
  _selector_.set_selection_list(selection_list);
  _selector_.reset_cursor_position();
  _selector_.reset_page();
  return true;

}
bool mod_browser::go_to_selected_directory(){
  std::string new_path = _current_directory_ + "/" + _selector_.get_selected_string();
  return change_directory(new_path);
}
bool mod_browser::go_back(){

  if(_current_directory_ == _base_folder_ or _current_directory_ == "/") return false; // already at maximum root

  auto folder_elements = toolbox::split_string(_current_directory_, "/");
  auto new_path = "/" + toolbox::join_vector_string(folder_elements, "/", 0, folder_elements.size()-1);
  if(not toolbox::do_path_is_folder(new_path)){
    std::cerr << "Can't go back, \"" << new_path << "\" is not a folder" << std::endl;
    return false;
  }
  return change_directory(new_path);

}
int mod_browser::get_path_depth(std::string& path_){
  if(not toolbox::do_string_starts_with_substring(path_, _base_folder_)) return -1; // sanity check
  int path_depth = (toolbox::split_string(path_, "/")).size() ;
  int base_depth = (toolbox::split_string(_base_folder_, "/")).size() ;
  return path_depth - base_depth;
}

