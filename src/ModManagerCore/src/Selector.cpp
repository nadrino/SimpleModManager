//
// Created by Nadrino on 12/09/2019.
//

#include "Selector.h"
#include "GlobalObjects.h"
#include <Toolbox.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>


void Selector::setCursorPosition(int cursorPosition_) {
  _cursorPosition_ = cursorPosition_;
}
void Selector::setEntryList(const std::vector<std::string>& entryTitleList_) {
  _entryList_.clear();
  _entryList_.reserve( entryTitleList_.size() );
  for( auto& title : entryTitleList_ ){
    _entryList_.emplace_back();
    _entryList_.back().title = title;
  }
}
void Selector::setMaxItemsPerPage(int maxItemsPerPage_){
  _maxItemsPerPage_ = maxItemsPerPage_;
}

void Selector::clearTags(){
  for( auto& entry : _entryList_ ){ entry.tag = ""; }
}
void Selector::setTag(size_t entryIndex_, const std::string &tag_){
  if( entryIndex_ >= _entryList_.size() ) return;
  _entryList_[entryIndex_].tag = tag_;
}
void Selector::setTagList(const std::vector<std::string>& tagList_){
  if( tagList_.size() != _entryList_.size() ) return;
  for( size_t iEntry = 0 ; iEntry < _entryList_.size() ; iEntry++ ){
    _entryList_[iEntry].tag = tagList_[iEntry];
  }
}
void Selector::setDescriptionList(const std::vector<std::vector<std::string>> &descriptionList_){
  if(descriptionList_.size() != _entryList_.size()) return;
  for( size_t iEntry = 0 ; iEntry < _entryList_.size() ; iEntry++ ){

  }
  _descriptionsList_ = descriptionList_;
  process_page_numbering();
}

int Selector::getNbPages() const {
  return _nbPages_;
}
int Selector::getCurrentPage() const {
  return _currentPage_;
}
int Selector::getCursorPosition() const {
  return _cursorPosition_;
}
int Selector::getSelectedEntryIndex() const {
  return _indexListForEachPage_[this->getCurrentPage()][this->getCursorPosition()];
}
int Selector::getEntry(const std::string &entryName_) const {
  return GenericToolbox::findElementIndex(entryName_, _entryList_);
}
const std::string & Selector::getTag(size_t entry_) const{
  return _tagList_[entry_];
}
const std::vector<std::string> & Selector::getSelectionList() const{
  return _entryList_;
}

void Selector::print() {

  std::string prefix_string;
  for(int i_entry = 0 ; i_entry < int(_indexListForEachPage_[_currentPage_].size()) ; i_entry++){
    int selection_list_entry = _indexListForEachPage_[_currentPage_][i_entry];

    prefix_string = "";
    if(i_entry == _cursorPosition_) prefix_string += _cursorMarker_; else prefix_string += " ";
    prefix_string += " ";
    GenericToolbox::Switch::Terminal::printLeftRight(
        prefix_string + _entryList_[selection_list_entry],
      _tagList_[selection_list_entry] + " ",
      (i_entry == _cursorPosition_ ? GenericToolbox::ColorCodes::blueBackground : "")
      );
    if(not _descriptionsList_[selection_list_entry].empty()){
      for(int i_desc_line = 0 ; i_desc_line < int(_descriptionsList_[selection_list_entry].size()) ; i_desc_line++){
        GenericToolbox::Switch::Terminal::printLeft(
            _descriptionsList_[selection_list_entry][i_desc_line],
            (i_entry == _cursorPosition_ ? GenericToolbox::ColorCodes::blueBackground : ""));
      }
    }

  }

}
void Selector::scanInputs(u64 kDown, u64 kHeld){

  // manage persistence
  if(kHeld == _previous_kHeld_){
    _holding_tiks_++;
  } else{
    _holding_tiks_ = 0;
  }
  _previous_kHeld_ = kHeld;

  if(kDown == 0 and kHeld == 0) return;

  if(kDown & HidNpadButton_AnyDown or (kHeld & HidNpadButton_AnyDown and _holding_tiks_ > 15 and _holding_tiks_%3 == 0)){
    incrementCursorPosition();
  } else if(kDown & HidNpadButton_AnyUp or (kHeld & HidNpadButton_AnyUp and _holding_tiks_ > 15 and _holding_tiks_%3 == 0)){
    decrementCursorPosition();
  } else if(kDown & HidNpadButton_AnyLeft){ // previous page
    previous_page();
  } else if(kDown & HidNpadButton_AnyRight){ // next page
    next_page();
  }

}
void Selector::reset_cursor_position(){
  _cursorPosition_ = _defaultCursorPosition_;
}
void Selector::reset_page(){
  _currentPage_ = 0;
}
void Selector::process_page_numbering(){
  int page_lines_buffer = 1;
  _indexListForEachPage_.clear();
  _indexListForEachPage_.emplace_back();
  for(int i_entry = 0 ; i_entry < int(_entryList_.size()) ; i_entry++){
    page_lines_buffer += 1 + int(_descriptionsList_[i_entry].size()); // space taken by i_entry
    if(page_lines_buffer == -1 or page_lines_buffer >= _maxItemsPerPage_){
      _indexListForEachPage_.emplace_back(); // next items will be displayed on the next page
      page_lines_buffer = 1; // shift, not 0
    }
//    std::stringstream ss;
//    ss << "page_lines_buffer=" << page_lines_buffer << " / i_entry=" << i_entry << "/" << _selection_list_[i_entry];
//    toolbox::print_left(ss.str());
//    GenericToolbox::Switch::Printout::makePause();
    _indexListForEachPage_.back().emplace_back(i_entry);
//    page_lines_buffer += 1 + int(_descriptions_list_[i_entry].size()); // space taken by i_entry
  }
  _nbPages_ = int(_indexListForEachPage_.size());
}
void Selector::incrementCursorPosition(){
  if(_entryList_.empty()){
    _cursorPosition_ = -1;
    return;
  }
  _cursorPosition_++; // increment
  // end of the page list
  if(_cursorPosition_ >= int(_indexListForEachPage_[_currentPage_].size())){
    next_page();
    _cursorPosition_ = 0;
  }
}
void Selector::decrementCursorPosition(){
  if(_entryList_.empty()){
    _cursorPosition_ = -1;
    return;
  }
  _cursorPosition_--; // decrement
  if(_cursorPosition_ < 0){
    previous_page();
    _cursorPosition_ = int(_indexListForEachPage_[_currentPage_].size()) - 1;
  }
}
void Selector::next_page(){
  _currentPage_++;
  if(_currentPage_ >= getNbPages()){
    _currentPage_ = 0;
  }
  else{
    _cursorPosition_ = 0;
  }
}
void Selector::previous_page(){
  _currentPage_--;
  if(_currentPage_ < 0){
    _currentPage_ = getNbPages() - 1;
  }
  else{
    _cursorPosition_ = 0;
  }
}

std::string Selector::getSelectedString(){
  if(_entryList_.empty()) return ""; // sanity check : emptiness
  if(getSelectedEntryIndex() < 0 or getSelectedEntryIndex() >= int(_entryList_.size()))
    return ""; // sanity check : cursor out of bounds
  return _entryList_[getSelectedEntryIndex()];
}

std::string Selector::ask_question(const std::string& question_, const std::vector<std::string>& answers_,
                                const std::vector<std::vector<std::string>>& descriptions_ ) {

  std::string answer;
  auto sel = Selector();

  int nb_lines_layout = 0;
  nb_lines_layout++; // toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());
  nb_lines_layout++; // std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
  nb_lines_layout += int(question_.size()) / GenericToolbox::Switch::Hardware::getTerminalWidth();
  nb_lines_layout++; // std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
  nb_lines_layout++; // std::cout << GenericToolbox::repeatString("*",toolbox::get_terminal_width());
  nb_lines_layout++; // toolbox::printLeft_right(" A: Select", "B: Back ");
  sel.setMaxItemsPerPage(GenericToolbox::Switch::Hardware::getTerminalHeight() - nb_lines_layout);

  sel.setEntryList(answers_);
  if(not descriptions_.empty() and descriptions_.size() == answers_.size()){
    sel.setDescriptionList(descriptions_);
  }

  u64 kDown = 1;
  while(appletMainLoop()){

    if(kDown != 0) {
      consoleClear();
      GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      std::cout << question_ << std::endl;
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      sel.print();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" A: Select", "B: Back ");
      consoleUpdate(nullptr);
    }

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);

    if     (kDown & HidNpadButton_AnyDown){
      sel.incrementCursorPosition();
    }
    else if(kDown & HidNpadButton_AnyUp){
      sel.decrementCursorPosition();
    }
    else if(kDown & HidNpadButton_A){
      answer = sel.getSelectedString();
      break;
    }
    else if(kDown & HidNpadButton_B){
      break;
    }

  }
  consoleClear();
  return answer;
}