//
// Created by Adrien Blanchet on 03/09/2019.
//

#include <browser.h>
#include <selector.h>
#include <toolbox.h>


#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <switch.h>
#include <algorithm>
#include <iomanip>


browser::browser(){

  reset();

}
browser::~browser(){

}

void browser::initialize(){
  _browser_selector_.initialize();
  change_directory(_base_folder_);
}
void browser::reset(){
  _nb_files_ = -1;
  _max_depth_ = -1;
  _base_folder_ = "/";
  _current_directory_ = _base_folder_;
  _selected_entry_name_ = "";
  _only_show_folders_ = false;
}

void browser::set_base_folder(std::string base_folder_){
  _base_folder_ = base_folder_;
}
void browser::set_max_depth(int max_depth_){
  _max_depth_ = max_depth_;
}
void browser::set_only_show_folders(bool only_show_folders_){
  _only_show_folders_ = only_show_folders_;
}

int browser::get_current_depth(){
  return _current_depth_;
}
int browser::get_max_depth(){
  return _max_depth_;
}
std::string browser::get_current_directory(){
  return _current_directory_;
}
std::string browser::get_selected_entry_name() {
  return _selected_entry_name_;
}
selector& browser::get_browser_selector(){
  return _browser_selector_;
}

void browser::print_ls(){

  std::cout << toolbox::red_bg << std::setw(toolbox::get_terminal_width()) << std::left;
  std::cout << "Current Folder : " + _current_directory_ << toolbox::reset_color;
//  std::cout << toolbox::red_bg << std::setw(toolbox::get_terminal_width()) << std::left << "Test" << toolbox::reset_color;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  _browser_selector_.print_selector();
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());

}
bool browser::change_directory(std::string new_directory_){

  if(not toolbox::do_path_is_folder(new_directory_)) return false;
  auto new_path_depth = get_path_depth(new_directory_);
  if(new_path_depth != -1 and new_path_depth > _max_depth_) return false;

  _current_directory_ = new_directory_;
  _current_depth_ = new_path_depth;
  std::vector<std::string> selection_list;
  if(not _only_show_folders_)selection_list = toolbox::get_list_of_entries_in_folder(_current_directory_);
  else selection_list = toolbox::get_list_of_folders_in_folder(_current_directory_);
  std::sort(selection_list.begin(), selection_list.end());
  _browser_selector_.set_selection_list(selection_list);
  _browser_selector_.reset_cursor_position();
  _browser_selector_.reset_page();
  return true;

}
bool browser::go_to_selected_directory(){
  std::string new_path = _current_directory_ + "/" + _browser_selector_.get_selected_string();
  return change_directory(new_path);
}
bool browser::go_back(){

  if(_current_directory_ == _base_folder_ or _current_directory_ == "/") return false; // already at maximum root

  auto folder_elements = toolbox::split_string(_current_directory_, "/");
  auto new_path = "/" + toolbox::join_vector_string(folder_elements, "/", 0, folder_elements.size()-1);
  if(not toolbox::do_path_is_folder(new_path)){
    std::cerr << "Can't go back, \"" << new_path << "\" is not a folder" << std::endl;
    return false;
  }
  return change_directory(new_path);

}
int browser::get_path_depth(std::string& path_){
  if(not toolbox::do_string_starts_with_substring(path_, _base_folder_)) return -1; // sanity check
  int path_depth = (toolbox::split_string(path_, "/")).size() ;
  int base_depth = (toolbox::split_string(_base_folder_, "/")).size() ;
  return path_depth - base_depth;
}

