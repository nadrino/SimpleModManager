//
// Created by Nadrino on 13/02/2020.
//

#include <Toolbox.h>
#include <ModsPreseter.h>
#include <Selector.h>
#include "GlobalObjects.h"

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

ModsPreseter::ModsPreseter() {
  reset();
}
ModsPreseter::~ModsPreseter() {
  reset();
}

void ModsPreseter::initialize() {

}
void ModsPreseter::reset() {

  _selected_mod_preset_index_ = -1;
  _mod_folder_ = "";
  _preset_file_path_ = "";
  _presets_list_.clear();
  _data_handler_.clear();

}

int ModsPreseter::get_selected_mod_preset_index(){
  return _selected_mod_preset_index_;
}
std::string ModsPreseter::get_selected_mod_preset(){
  if(_selected_mod_preset_index_ >= 0 and _selected_mod_preset_index_ < int(_presets_list_.size())){
    return _presets_list_[_selected_mod_preset_index_];
  } else{
    return "";
  }
}
std::vector<std::string> ModsPreseter::get_mods_list(std::string preset_) {
  return _data_handler_[preset_];
}
std::vector<std::string>& ModsPreseter::get_presets_list(){
  return _presets_list_;
}

void ModsPreseter::read_parameter_file(std::string mod_folder_) {

  if(mod_folder_.empty()) mod_folder_ = _mod_folder_;

  reset();
  _mod_folder_ = mod_folder_;
  _preset_file_path_ = mod_folder_ + "/mod_presets.conf";

  // check if file exist
  auto lines = GenericToolbox::dumpFileAsVectorString(_preset_file_path_);
  std::string current_preset = "";
  for(auto &line : lines){
    if(line[0] == '#') continue;

    auto line_elements = GenericToolbox::splitString(line, "=");
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
      if(_selected_mod_preset_index_ == -1) _selected_mod_preset_index_ = 0;
      if (not GenericToolbox::doesElementIsInVector(current_preset, _presets_list_)){
        _presets_list_.emplace_back(current_preset);
      }
    } else {
      _data_handler_[current_preset].emplace_back(line_elements[1]);
    }
  }

  fill_selector();

}
void ModsPreseter::recreate_preset_file() {

  std::stringstream ss;

  ss << "# This is a config file" << std::endl;
  ss << std::endl;
  ss << std::endl;

  for(auto const &preset : _presets_list_){
    ss << "########################################" << std::endl;
    ss << "# mods preset name" << std::endl;
    ss << "preset = " << preset << std::endl;
    ss << std::endl;
    ss << "# mods list" << std::endl;
    for(int i_entry = 0 ; i_entry < int(_data_handler_[preset].size()) ; i_entry++){
      ss << "mod" << i_entry << " = " << _data_handler_[preset][i_entry] << std::endl;
    }
    ss << "########################################" << std::endl;
    ss << std::endl;
  }

  std::string data = ss.str();
  GenericToolbox::dumpStringInFile(_preset_file_path_, data);

}
void ModsPreseter::select_mod_preset() {

  fill_selector();

  bool is_first_loop = true;
  while(appletMainLoop()){

    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    u64 kHeld = padGetButtons(&GlobalObjects::gPad);
    _selector_.scan_inputs(kDown, kHeld);
    if(kDown & HidNpadButton_B){
      break;
    }
    else if(kDown & HidNpadButton_A and not _presets_list_.empty()){
      _selected_mod_preset_index_ = _selector_.get_selected_entry();
      return;
    }
    else if(kDown & HidNpadButton_X and not _presets_list_.empty()){
      std::string answer = Selector::ask_question(
        "Are you sure you want to remove this preset ?",
        std::vector<std::string>({"Yes", "No"})
      );
      if(answer == "No") continue;
      delete_mod_preset(_presets_list_[_selector_.get_selected_entry()]);
    }
    else if(kDown & HidNpadButton_Plus){
      create_new_preset();
      fill_selector();
      recreate_preset_file();
      read_parameter_file(_mod_folder_);
    }
    else if(kDown & HidNpadButton_Y){
      edit_preset(_selector_.get_selected_string(), _data_handler_[_selector_.get_selected_string()]);
      fill_selector();
      recreate_preset_file();
      read_parameter_file(_mod_folder_);
    }

    if(kDown != 0 or kHeld != 0 or is_first_loop){
      is_first_loop = false;
      consoleClear();
      GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
      std::cout << GenericToolbox::ColorCodes::redBackground << std::setw(GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::left;
      std::cout << "Select mod preset" << GenericToolbox::ColorCodes::resetColor;
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      _selector_.print_selector();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("  Page (" + std::to_string(_selector_.get_current_page() + 1) + "/" + std::to_string(_selector_.get_nb_pages()) + ")");
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" A : Select mod preset", " X : Delete mod preset ");
      GenericToolbox::Switch::Terminal::printLeftRight(" Y : Edit preset", "+ : Create a preset ");
      GenericToolbox::Switch::Terminal::printLeft(" B : Go back");
      consoleUpdate(nullptr);
    }

  }
}
void ModsPreseter::create_new_preset(){

  int preset_id = 1;
  std::string default_preset_name;
  do{
    default_preset_name = "preset-" + std::to_string(_presets_list_.size()+preset_id);
    preset_id++;
  } while(GenericToolbox::doesElementIsInVector(default_preset_name, _presets_list_));

  std::vector<std::string> selected_mods_list;
  edit_preset(default_preset_name, selected_mods_list);

}
void ModsPreseter::delete_mod_preset(std::string preset_name_){

  int itemToDeleteIndex = _selector_.get_entry(preset_name_);
  if(itemToDeleteIndex == -1) return;

  _data_handler_[preset_name_].resize(0);
  _presets_list_.erase(_presets_list_.begin() + itemToDeleteIndex);
  fill_selector();
  recreate_preset_file();
  read_parameter_file(_mod_folder_);

}
void ModsPreseter::edit_preset(std::string preset_name_, std::vector<std::string> selected_mods_list_) {

  std::vector<std::string> mods_list = GenericToolbox::getListOfFilesInSubFolders(_mod_folder_);
  std::sort(mods_list.begin(), mods_list.end());
  Selector sel;
  sel.set_selection_list(mods_list);

  for(int i_entry = 0 ; i_entry < int(selected_mods_list_.size()) ; i_entry++){
    for(int j_entry = 0 ; j_entry < int(mods_list.size()) ; j_entry++){

      if(selected_mods_list_[i_entry] == mods_list[j_entry]){
        std::string new_tag = sel.get_tag(j_entry);
        if(not new_tag.empty()) new_tag += " & ";
        new_tag += "#" + std::to_string(i_entry);
        sel.set_tag(j_entry, new_tag);
        break;
      }

    }
  }

  bool is_first_loop = true;
  while(appletMainLoop()){

    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    u64 kHeld = padGetButtons(&GlobalObjects::gPad);
    sel.scan_inputs(kDown, kHeld);
    if(kDown & HidNpadButton_A){
      selected_mods_list_.emplace_back(sel.get_selected_string());
      std::string new_tag = sel.get_tag(sel.get_selected_entry());
      if(not new_tag.empty()) new_tag += " & ";
      new_tag += "#" + std::to_string(selected_mods_list_.size());
      sel.set_tag(sel.get_selected_entry(), new_tag);
    } else if(kDown & HidNpadButton_X){
      int original_size = selected_mods_list_.size();
      // in decreasing order because we want to remove the last occurrence of the mod first
      for(int i_entry = int(selected_mods_list_.size()) - 1 ; i_entry >= 0 ; i_entry--){
        if(sel.get_selected_string() == selected_mods_list_[i_entry]){
          selected_mods_list_.erase(selected_mods_list_.begin() + i_entry);

          // reprocessing all tags
          for(int j_mod = 0 ; j_mod < int(mods_list.size()) ; j_mod++){
            sel.set_tag(j_mod, "");
          }
          for(int j_entry = 0 ; j_entry < int(selected_mods_list_.size()) ; j_entry++){
            for(int j_mod = 0 ; j_mod < int(mods_list.size()) ; j_mod++){
              if(selected_mods_list_[j_entry] == mods_list[j_mod]){
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
      selected_mods_list_.resize(original_size-1);
    } else if(kDown & HidNpadButton_Plus){
      break;
    } else if(kDown & HidNpadButton_B){
      return;
    }

    if(kDown != 0 or kHeld != 0 or is_first_loop){
      consoleClear();
      GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
      std::cout << GenericToolbox::ColorCodes::redBackground << std::setw(GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::left;
      std::string header_title = "Creating preset : " + preset_name_ + ". Select the mods you want.";
      std::cout << header_title << GenericToolbox::ColorCodes::resetColor;
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      sel.print_selector();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" A : Add mod", "X : Cancel mod ");
      GenericToolbox::Switch::Terminal::printLeftRight(" + : SAVE", "B : Abort / Go back ");
      consoleUpdate(nullptr);
    }
  }

  _data_handler_[preset_name_].clear();
  _data_handler_[preset_name_].resize(0);

  int preset_index = -1;
  for(int i_index = 0 ; i_index < int(_presets_list_.size()) ; i_index++){
    if(_presets_list_[i_index] == preset_name_) preset_index = i_index;
  }

  if(preset_index == -1){
    preset_index = _presets_list_.size();
    _presets_list_.emplace_back(preset_name_);
  }


  preset_name_ = GenericToolbox::Switch::UI::openKeyboardUi(preset_name_);
  _presets_list_[preset_index] = preset_name_;

  for(int i_entry = 0 ; i_entry < int(selected_mods_list_.size()) ; i_entry++){
    _data_handler_[preset_name_].emplace_back(selected_mods_list_[i_entry]);
  }

  // Check for conflicts
  show_conflicted_files(preset_name_);


}
void ModsPreseter::show_conflicted_files(std::string &preset_name_) {

  consoleClear();

  GenericToolbox::Switch::Terminal::printLeft("Scanning preset files...", GenericToolbox::ColorCodes::magentaBackground);
  consoleUpdate(nullptr);

  std::vector<std::string> complete_files_list;
  std::map<std::string, long int> files_size_map;
  std::map<std::string, std::string> conflict_files_map;

  for(int i_entry = 0 ; i_entry < int(_data_handler_[preset_name_].size()) ; i_entry++){

    GenericToolbox::Switch::Terminal::printLeft(" > Getting files for the mod: " + _data_handler_[preset_name_][i_entry], GenericToolbox::ColorCodes::magentaBackground);
    consoleUpdate(nullptr);

    std::string mod_folder_path = _mod_folder_ + "/" + _data_handler_[preset_name_][i_entry];
    auto mod_files_path_list = GenericToolbox::getListOfFilesInSubFolders(mod_folder_path);
    for(auto& mod_file_path: mod_files_path_list){
      std::string mod_file_full_path = mod_folder_path + "/" + mod_file_path;
      files_size_map[mod_file_path] = GenericToolbox::getFileSize(mod_file_full_path); // will overwrite when conflict
      if(not GenericToolbox::doesElementIsInVector(mod_file_path,complete_files_list)){
        complete_files_list.emplace_back(mod_file_path);
      }
      else{
        conflict_files_map[mod_file_path] = _data_handler_[preset_name_][i_entry];
      }
    }

  }

  long int total_SD_size = 0;
  for(auto& file_size : files_size_map){
    total_SD_size += file_size.second;
  }
  std::string total_SD_size_str = GenericToolbox::parseSizeUnits(total_SD_size);

  std::vector<std::string> sel_conflict_file_list;
  std::vector<std::string> tag_mod_used_list;
  if(conflict_files_map.empty()){
    sel_conflict_file_list.emplace_back("No conflict has been found.");
    tag_mod_used_list.emplace_back("");
  }
  else{
    for(auto& conflict: conflict_files_map){
      sel_conflict_file_list.emplace_back(GenericToolbox::getFileNameFromFilePath(conflict.first));
      tag_mod_used_list.emplace_back("-> \"" + conflict.second + "\" will be used.");
    }
  }


  Selector sel;
  sel.set_selection_list(sel_conflict_file_list);
  sel.set_tags_list(tag_mod_used_list);
  sel.set_max_items_per_page(GenericToolbox::Switch::Hardware::getTerminalHeight() - 9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
      GenericToolbox::Switch::Terminal::printLeft("Conflicted files for the preset \"" + preset_name_ + "\":", GenericToolbox::ColorCodes::redBackground);
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      sel.print_selector();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("Total size of the preset:" + total_SD_size_str, GenericToolbox::ColorCodes::greenBackground);
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("Page (" + std::to_string(sel.get_current_page() + 1) + "/" + std::to_string(sel.get_nb_pages()) + ")");
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft(" A : OK");
      if(sel.get_nb_pages() > 1) GenericToolbox::Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
      consoleUpdate(nullptr);
    }

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);
    kHeld = padGetButtons(&GlobalObjects::gPad);

    if (kDown & HidNpadButton_A) {
      break; // break in order to return to hbmenu
    }

    sel.scan_inputs(kDown, kHeld);

  }

}

std::map<std::string, std::vector<std::string>> ModsPreseter::get_conflicts_with_other_mods(const std::string& mod_name_) {

  GenericToolbox::Switch::Terminal::printLeft("Searching for conflicts with " + mod_name_, GenericToolbox::ColorCodes::magentaBackground);
  consoleUpdate(nullptr);
  std::map<std::string, std::vector<std::string>> conflicts_map;

  std::string mod_folder_path = _mod_folder_ + "/" + mod_name_;
  GenericToolbox::Switch::Terminal::printLeft(" > Getting list of files for " + mod_name_, GenericToolbox::ColorCodes::magentaBackground);
  consoleUpdate(nullptr);
  auto mod_files_path_list = GenericToolbox::getListOfFilesInSubFolders(mod_folder_path);

  auto other_mods_list = GenericToolbox::getListOfFilesInSubFolders(_mod_folder_);

  for(auto& other_mod_name: other_mods_list){

    if(other_mod_name == mod_name_) continue;

    conflicts_map[other_mod_name] = std::vector<std::string>();

    std::string other_mod_folder_path = _mod_folder_ + "/" + other_mod_name;
    GenericToolbox::Switch::Terminal::printLeft(" > Scanning conflicts with " + other_mod_name, GenericToolbox::ColorCodes::magentaBackground);
    consoleUpdate(nullptr);
    auto other_mod_files_path_list = GenericToolbox::getListOfFilesInSubFolders(other_mod_folder_path);
    for(auto& other_mod_file_path: other_mod_files_path_list){
      if(GenericToolbox::doesElementIsInVector(other_mod_file_path, mod_files_path_list)){
        // if the two files are the same, no need to consider them as conflict
        if(not GenericToolbox::Switch::IO::doFilesAreIdentical(
          other_mod_folder_path + "/" + other_mod_file_path,
          mod_folder_path + "/" + other_mod_file_path
          )){
          conflicts_map[other_mod_name].emplace_back(other_mod_file_path);
        }
      }
    }
  }

  return conflicts_map;

}

void ModsPreseter::select_previous_mod_preset(){
  if(_selected_mod_preset_index_ == -1) return;
  _selected_mod_preset_index_--;
  if(_selected_mod_preset_index_ < 0) _selected_mod_preset_index_ = int(_presets_list_.size()) - 1;
}
void ModsPreseter::select_next_mod_preset(){
  if(_selected_mod_preset_index_ == -1) return;
  _selected_mod_preset_index_++;
  if(_selected_mod_preset_index_ >= int(_presets_list_.size())) _selected_mod_preset_index_ = 0;
}

void ModsPreseter::fill_selector(){
  _selector_.reset();
  if(not _presets_list_.empty()){
    _selector_.set_selection_list(_presets_list_);
    for(int i_preset = 0 ; i_preset < int(_presets_list_.size()) ; i_preset++){
      std::vector<std::string> description_lines;
      description_lines.reserve(int(_data_handler_[_presets_list_[i_preset]].size()));
      for(int i_entry = 0 ; i_entry < int(_data_handler_[_presets_list_[i_preset]].size()) ; i_entry++){
        description_lines.emplace_back("  | " + _data_handler_[_presets_list_[i_preset]][i_entry]);
      }
      _selector_.set_description(i_preset, description_lines);
    }
  }
  else {
    std::vector<std::string> empty_list;
    empty_list.emplace_back("NO MODS PRESETS");
    _selector_.set_selection_list(empty_list);
  }

}

std::map<std::string, std::vector<std::string>> &ModsPreseter::get_data_handler() {
  return _data_handler_;
}

void ModsPreseter::setSelector(Selector selector) {
  _selector_ = selector;
}
