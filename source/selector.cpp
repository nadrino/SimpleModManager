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
selector::~selector() {

}

void selector::initialize(){

}
void selector::reset(){

  set_cursor_marker(">");
  set_default_cursor_position(0);
  reset_cursor_position();
  _max_items_per_page_ = 27;
  _current_page_ = 0;

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

int selector::get_nb_pages(){
  return 1 + int(_selection_list_.size()-1)/_max_items_per_page_;
}
int selector::get_current_page(){
  return _current_page_;
}
int selector::get_selected_entry(){
  return _current_page_*_max_items_per_page_ + _cursor_position_;
}
std::vector<std::string> selector::get_selection_list(){
  return _selection_list_;
}

void selector::print_selector() {

  std::string prefix_string;
  for(int i_entry = 0 ; i_entry < _max_items_per_page_ ; i_entry++){
    int current_entry = _current_page_*_max_items_per_page_ + i_entry;
    if(current_entry >= int(_selection_list_.size())) break;
    prefix_string = "";
    if(i_entry == _cursor_position_) prefix_string += _cursor_marker_;
    else prefix_string += " ";
    prefix_string += " ";
    std::string selection_element = _selection_list_[current_entry];
    std::string spaces = "";

    int total_line_length = prefix_string.size() + selection_element.size() + _tags_list_[current_entry].size() + 2;
    if(total_line_length > consoleGetDefault()->consoleWidth){ // then cut the mod title
      selection_element = selection_element.substr(
          0,
          selection_element.size() - (total_line_length - consoleGetDefault()->consoleWidth)
          );
    } else { // then increase space
      for(int i_space = 0 ; i_space < (consoleGetDefault()->consoleWidth - total_line_length) ; i_space++ ){
        spaces += " ";
      }
    }

    if(i_entry == _cursor_position_) std::cout << toolbox::blue_bg;
    std::cout << prefix_string << selection_element << spaces << _tags_list_[current_entry] << "  " << toolbox::reset_color;
  }

}
void selector::reset_cursor_position(){
  _cursor_position_ = _default_cursor_position_;
}
void selector::reset_page(){
  _current_page_ = 0;
}
void selector::reset_tags_list(){
  _tags_list_.resize(0);
  _tags_list_.resize(_selection_list_.size());
}
void selector::increment_cursor_position(){
  if(_selection_list_.empty()){
    _cursor_position_ = -1;
    return;
  }
  _cursor_position_++; // increment
  if(
      _cursor_position_ >= _max_items_per_page_                                                    // end of the page list
      or _cursor_position_ >= int(_selection_list_.size()) - _current_page_*_max_items_per_page_   // end of the list
      ){
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
    if(_current_page_+1 < get_nb_pages()){ // Not the last page
      _cursor_position_ = _max_items_per_page_-1; // last page item
    } else {                               // last page
      _cursor_position_ = int(_selection_list_.size()) - _current_page_*_max_items_per_page_ - 1; // last item
    }
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
