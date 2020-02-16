//
// Created by Adrien BLANCHET on 13/02/2020.
//

#include <toolbox.h>
#include <mods_preseter.h>
#include <fstream>
//#include <switch/services/applet.h>
#include <switch.h>
#include <selector.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

mods_preseter::mods_preseter() {
  reset();
}
mods_preseter::~mods_preseter() {
  reset();
}

void mods_preseter::initialize() {

}
void mods_preseter::reset() {

  _mod_folder_ = "";
  _preset_file_path_ = "";
  _selected_mod_preset_ = "";
  _presets_list_.clear();
  _data_handler_.clear();

}

std::string mods_preseter::get_selected_mod_preset(){
  return _selected_mod_preset_;
}
std::vector<std::string> mods_preseter::get_mods_list(std::string preset_) {
  return _data_handler_[preset_];
}

void mods_preseter::read_parameter_file(std::string mod_folder_) {

  reset();
  _mod_folder_ = mod_folder_;
  _preset_file_path_ = mod_folder_ + "/mod_presets.conf";

  // check if file exist
  if(toolbox::do_path_is_file(_preset_file_path_)){

    std::ifstream parameter_file;
    parameter_file.open (_preset_file_path_.c_str());

    std::string current_preset = "";
    std::string line;
    while( std::getline(parameter_file, line) ){

      if(line[0] == '#') continue;

      auto line_elements = toolbox::split_string(line, "=");
      if(line_elements.size() != 2) continue;

      // clean up for extra spaces characters
      for(auto &element : line_elements){
        while(element[0] == ' '){
          element.erase(element.begin());
        }
        while(element[element.size()-1] == ' '){
          element.erase(element.end()-1);
        }
      }

      if(line_elements[0] == "preset"){
        current_preset = line_elements[1];
        if(_selected_mod_preset_.empty()) _selected_mod_preset_ = current_preset;
        if (not toolbox::do_string_in_vector(current_preset, _presets_list_)){
          _presets_list_.emplace_back(current_preset);
        }
      } else {
        _data_handler_[current_preset].emplace_back(line_elements[1]);
      }
    }
    parameter_file.close();

  }

}
void mods_preseter::recreate_preset_file() {

  if(_presets_list_.empty()){
    toolbox::rm_file(_preset_file_path_);
    return;
  }

  std::ofstream parameter_file;
  parameter_file.open (_preset_file_path_.c_str());

  parameter_file << "# This is a config file" << std::endl;
  parameter_file << std::endl;
  parameter_file << std::endl;

  for(auto const &preset : _presets_list_){
    parameter_file << "########################################" << std::endl;
    parameter_file << "# mods preset name" << std::endl;
    parameter_file << "preset = " << preset << std::endl;
    parameter_file << std::endl;
    parameter_file << "# mods list" << std::endl;
    for(int i_entry = 0 ; i_entry < int(_data_handler_[preset].size()) ; i_entry++){
      parameter_file << "mod" << i_entry << " = " << _data_handler_[preset][i_entry] << std::endl;
    }
    parameter_file << "########################################" << std::endl;
    parameter_file << std::endl;
  }

}
void mods_preseter::select_mod_preset() {

  selector sel = fill_selector();

  bool is_first_loop = true;
  while(appletMainLoop()){

    hidScanInput();
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
    sel.scan_inputs(kDown, kHeld);
    if(kDown & KEY_B){
      break;
    } else if(kDown & KEY_A and not _presets_list_.empty()){
      _selected_mod_preset_ = sel.get_selected_string();
      return;
    } else if(kDown & KEY_X and not _presets_list_.empty()){
      std::string answer = toolbox::ask_question(
        "Are you sure you want to remove this preset ?",
        std::vector<std::string>({"Yes","No"})
        );
      if(answer == "No") continue;
      _data_handler_[_presets_list_[sel.get_selected_entry()]].resize(0);
      _presets_list_.erase(_presets_list_.begin() + sel.get_selected_entry());
      sel = fill_selector();
      recreate_preset_file();
      read_parameter_file(_mod_folder_);
    } else if(kDown & KEY_PLUS){
      create_preset();
      sel = fill_selector();
      recreate_preset_file();
      read_parameter_file(_mod_folder_);
    }

    if(kDown != 0 or kHeld != 0 or is_first_loop){
      is_first_loop = false;
      consoleClear();
      toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());
      std::cout << toolbox::red_bg << std::setw(toolbox::get_terminal_width()) << std::left;
      std::cout << "Select mod preset" << toolbox::reset_color;
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      sel.print_selector();
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      toolbox::print_left_right("A : Select mod preset", "X : Delete mod preset ");
      toolbox::print_left("+ : Create a preset");
      toolbox::print_left("B : Go back");
      consoleUpdate(nullptr);
    }

  }
}
void mods_preseter::create_preset(){

  int preset_id = 1;
  std::string default_preset_name;
  do{
    default_preset_name = "preset-" + std::to_string(_presets_list_.size()+preset_id);
    preset_id++;
  } while(toolbox::do_string_in_vector(default_preset_name, _presets_list_));


  std::vector<std::string> mods_list = toolbox::get_list_of_folders_in_folder(_mod_folder_);
  std::sort(mods_list.begin(), mods_list.end());
  selector sel;
  sel.set_selection_list(mods_list);

  std::vector<std::string> selected_mods_list;

  bool is_first_loop = true;
  while(appletMainLoop()){

    hidScanInput();
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
    sel.scan_inputs(kDown, kHeld);
    if(kDown & KEY_A){
      selected_mods_list.emplace_back(sel.get_selected_string());
      std::string new_tag = sel.get_tag(sel.get_selected_entry());
      if(not new_tag.empty()) new_tag += " & ";
      new_tag += "#" + std::to_string(selected_mods_list.size());
      sel.set_tag(sel.get_selected_entry(), new_tag);
    } else if(kDown & KEY_X){
      for(int i_entry = int(selected_mods_list.size())-1 ; i_entry >= 0 ; i_entry--){
        if(sel.get_selected_string() == selected_mods_list[i_entry]){
          selected_mods_list.erase(selected_mods_list.begin() + i_entry);

          // reprocessing all tags
          for(int j_mod = 0 ; j_mod < int(mods_list.size()) ; j_mod++){
            sel.set_tag(j_mod, "");
          }
          for(int j_entry = 0 ; j_entry < int(selected_mods_list.size()) ; j_entry++){
            for(int j_mod = 0 ; j_mod < int(mods_list.size()) ; j_mod++){
              if(selected_mods_list[j_entry] == mods_list[j_mod]){
                std::string new_tag = sel.get_tag(j_mod);
                if(not new_tag.empty()) new_tag += " & ";
                new_tag += "#" + std::to_string(j_entry+1);
                sel.set_tag(j_mod, new_tag);
              }
            }
          }
          break; // for loop
        }
      }
    } else if(kDown & KEY_PLUS){
      break;
    } else if(kDown & KEY_B){
      return;
    }

    if(kDown != 0 or kHeld != 0 or is_first_loop){
      consoleClear();
      toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());
      std::cout << toolbox::red_bg << std::setw(toolbox::get_terminal_width()) << std::left;
      std::string header_title = "Creating preset : " + default_preset_name + ". Select the mods you want.";
      std::cout << header_title << toolbox::reset_color;
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      sel.print_selector();
      std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
      toolbox::print_left_right("L : Previous page", "R : Next page ");
      toolbox::print_left_right("A : Add mod", "X : Cancel mod ");
      toolbox::print_left_right("+ : SAVE", "B : Abort / Go back ");
      consoleUpdate(nullptr);
    }

  }

  _presets_list_.emplace_back(default_preset_name);
  for(int i_entry = 0 ; i_entry < int(selected_mods_list.size()) ; i_entry++){
    _data_handler_[default_preset_name].emplace_back(selected_mods_list[i_entry]);
  }


}

selector mods_preseter::fill_selector(){
  selector sel;
  if(not _presets_list_.empty()){
    sel.set_selection_list(_presets_list_);
    for(int i_preset = 0 ; i_preset < int(_presets_list_.size()) ; i_preset++){
      std::vector<std::string> description_lines;
      for(int i_entry = 0 ; i_entry < int(_data_handler_[_presets_list_[i_preset]].size()) ; i_entry++){
        description_lines.emplace_back("  | " + _data_handler_[_presets_list_[i_preset]][i_entry]);
      }
      sel.set_description(i_preset, description_lines);
    }
  } else {
    std::vector<std::string> empty_list;
    empty_list.emplace_back("NO MODS PRESETS");
    sel.set_selection_list(empty_list);
  }
  return sel;
}
