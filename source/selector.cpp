//
// Created by Adrien Blanchet on 12/09/2019.
//

#include "selector.h"
#include <toolbox.h>

#include <iostream>
#include <switch.h>

selector::selector() {

  reset();

}
selector::~selector() = default;

void selector::initialize(){

  // nothing to initialize

}
void selector::reset(){

  set_cursor_marker(">");
  set_default_cursor_position(0);
  reset_cursor_position();
  _max_items_per_page_ = 27;
  _nb_pages_ = 0;
  _current_page_ = 0;

  _selection_list_.clear();
  _tags_list_.clear();
  _descriptions_list_.clear();

}

void selector::set_default_cursor_position(int default_cursor_position_){
  _default_cursor_position_ = default_cursor_position_;
}
void selector::set_cursor_position(int cursor_position_) {
  _cursor_position_ = cursor_position_;
}
void selector::set_selection_list(std::vector<std::string> selection_list_) {
  _selection_list_ = selection_list_;
  reset_tags_list();
  reset_description_list();
}
void selector::set_cursor_marker(std::string cursor_marker_) {
  _cursor_marker_ = cursor_marker_;
}
void selector::set_max_items_per_page(int max_items_per_page_){
  _max_items_per_page_ = max_items_per_page_;
}

void selector::set_tag(int entry_, std::string tag_){
  if(entry_ < 0 or entry_ >= int(_tags_list_.size())) return;
  _tags_list_[entry_] = tag_;
}
void selector::set_description(int entry_, std::vector<std::string> description_lines_){
  if(entry_ < 0 or entry_ >= int(_descriptions_list_.size())) return;
  _descriptions_list_[entry_] = description_lines_;
  process_page_numbering();
}

int selector::get_nb_pages(){
  return _nb_pages_;
}
int selector::get_current_page(){
  return _current_page_;
}
int selector::get_selected_entry(){
  return _item_list_for_each_page_[_current_page_][_cursor_position_];
}
std::string selector::get_tag(int entry_){
  return _tags_list_[entry_];
}
std::vector<std::string> selector::get_selection_list(){
  return _selection_list_;
}

void selector::print_selector() {

  std::string prefix_string;
  std::string color;
  for(int i_entry = 0 ; i_entry < int(_item_list_for_each_page_[_current_page_].size()) ; i_entry++){
    int selection_list_entry = _item_list_for_each_page_[_current_page_][i_entry];

    prefix_string = "";
    if(i_entry == _cursor_position_) prefix_string += _cursor_marker_; else prefix_string += " ";
    prefix_string += " ";
    if(i_entry == _cursor_position_) color = toolbox::blue_bg; else color = "";
    _selection_list_[selection_list_entry];
    toolbox::print_left_right(
      prefix_string + _selection_list_[selection_list_entry],
      _tags_list_[selection_list_entry] + " ",
      color
      );
    if(not _descriptions_list_[selection_list_entry].empty()){
      for(int i_desc_line = 0 ; i_desc_line < int(_descriptions_list_[selection_list_entry].size()) ; i_desc_line++){
        toolbox::print_left(_descriptions_list_[selection_list_entry][i_desc_line], color);
      }
    }

  }


//  for(int i_entry = 0 ; i_entry < _max_items_per_page_ ; i_entry++){
//    // page management
//    int current_entry = _current_page_*_max_items_per_page_ + i_entry;
//    if(current_entry >= int(_selection_list_.size())) break;
//
//    // formatting
//    prefix_string = "";
//    if(i_entry == _cursor_position_) prefix_string += _cursor_marker_;
//    else prefix_string += " ";
//    prefix_string += " ";
//    std::string element_title = _selection_list_[current_entry];
//    std::string spaces = "";
//    int total_line_length = prefix_string.size() + element_title.size() + _tags_list_[current_entry].size() + 2;
//    if(total_line_length > consoleGetDefault()->consoleWidth){ // then cut the mod title
//      element_title = element_title.substr(
//        0,
//        element_title.size() - (total_line_length - consoleGetDefault()->consoleWidth)
//          );
//    } else { // then increase space
//      for(int i_space = 0 ; i_space < (consoleGetDefault()->consoleWidth - total_line_length) ; i_space++ ){
//        spaces += " ";
//      }
//    }
//
//    // actual printing
//    if(i_entry == _cursor_position_) std::cout << toolbox::blue_bg;
//    std::cout << prefix_string << element_title << spaces << _tags_list_[current_entry] << "  " << toolbox::reset_color;
//    if(not _descriptions_list_[current_entry].empty()){
//      for(int i_desc_line = 0 ; i_desc_line < int(_descriptions_list_[current_entry].size()) ; i_desc_line++){
//        if(i_entry == _cursor_position_) toolbox::print_left(_descriptions_list_[current_entry][i_desc_line], toolbox::blue_bg);
//        else toolbox::print_left(_descriptions_list_[current_entry][i_desc_line]);
//      }
//    }
//  }

}
void selector::scan_inputs(u64 kDown, u64 kHeld){

  // manage persistence
  if(kHeld == _previous_kHeld_){
    _holding_tiks_++;
  } else{
    _holding_tiks_ = 0;
  }
  _previous_kHeld_ = kHeld;

  if(kDown == 0 and kHeld == 0) return;

  if(kDown & KEY_DOWN or (kHeld & KEY_DOWN and _holding_tiks_ > 15 and _holding_tiks_%3 == 0)){
    increment_cursor_position();
  } else if(kDown & KEY_UP or (kHeld & KEY_UP and _holding_tiks_ > 15 and _holding_tiks_%3 == 0)){
    decrement_cursor_position();
  } else if(kDown & KEY_L){ // previous page
    previous_page();
  } else if(kDown & KEY_R){ // next page
    next_page();
  }

}
void selector::reset_cursor_position(){
  _cursor_position_ = _default_cursor_position_;
}
void selector::reset_page(){
  _current_page_ = 0;
}
void selector::reset_tags_list(){
  _tags_list_.clear();
  _tags_list_.resize(_selection_list_.size());
}
void selector::reset_description_list(){
  _descriptions_list_.clear();
  _descriptions_list_.resize(_selection_list_.size());
  process_page_numbering(); // reset
}
void selector::process_page_numbering(){
  int page_lines_buffer = -1;
  _item_list_for_each_page_.clear();
  for(int i_entry = 0 ; i_entry < int(_selection_list_.size()) ; i_entry++){
    if(page_lines_buffer == -1 or page_lines_buffer >= _max_items_per_page_){
      _item_list_for_each_page_.emplace_back(); // next items will be displayed on the next page
      page_lines_buffer = 1; // shift, not 0
    }
    _item_list_for_each_page_.back().emplace_back(i_entry);
    page_lines_buffer += 1 + int(_descriptions_list_[i_entry].size()); // space taken by i_entry
  }
  _nb_pages_ = int(_item_list_for_each_page_.size());
}
void selector::increment_cursor_position(){
  if(_selection_list_.empty()){
    _cursor_position_ = -1;
    return;
  }
  _cursor_position_++; // increment
  // end of the page list
  if(_cursor_position_ >= int(_item_list_for_each_page_[_current_page_].size())){
    next_page();
    _cursor_position_ = 0;
  }
}
void selector::decrement_cursor_position(){
  if(_selection_list_.empty()){
    _cursor_position_ = -1;
    return;
  }
  _cursor_position_--; // decrement
  if(_cursor_position_ < 0){
    previous_page();
    _cursor_position_ = int(_item_list_for_each_page_[_current_page_].size()) - 1;
  }
}
void selector::next_page(){
  _current_page_++;
  if(_current_page_ >= get_nb_pages()) _current_page_ = 0;
  _cursor_position_ = 0;
}
void selector::previous_page(){
  _current_page_--;
  if(_current_page_ < 0) _current_page_ = get_nb_pages() - 1;
  _cursor_position_ = 0;
}

std::string selector::get_selected_string(){
  if(_selection_list_.empty()) return ""; // sanity check : emptiness
  if(get_selected_entry() < 0 or get_selected_entry() >= int(_selection_list_.size()))
    return ""; // sanity check : cursor out of bounds
  return _selection_list_[get_selected_entry()];
}
