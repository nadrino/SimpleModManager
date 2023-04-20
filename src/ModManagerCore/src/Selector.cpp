//
// Created by Nadrino on 12/09/2019.
//

#include "Selector.h"
#include "GlobalObjects.h"
#include <Toolbox.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>

using namespace GenericToolbox::Switch;

// non native setters
void Selector::setEntryList(const std::vector<std::string>& entryTitleList_) {
  this->invalidateCache();
  _cursorPosition_ = 0;
  _entryList_.clear();
  _entryList_.reserve( entryTitleList_.size() );
  for( auto& title : entryTitleList_ ){
    _entryList_.emplace_back();
    _entryList_.back().title = title;
  }
}
void Selector::setTag(size_t entryIndex_, const std::string &tag_){
  this->invalidateCache();
  if( entryIndex_ >= _entryList_.size() ) return;
  _entryList_[entryIndex_].tag = tag_;
}
void Selector::setTagList(const std::vector<std::string>& tagList_){
  this->invalidateCache();
  if( tagList_.size() != _entryList_.size() ) return;
  for( size_t iEntry = 0 ; iEntry < _entryList_.size() ; iEntry++ ){
    _entryList_[iEntry].tag = tagList_[iEntry];
  }
}
void Selector::setDescriptionList(const std::vector<std::vector<std::string>> &descriptionList_){
  this->invalidateCache();
  if(descriptionList_.size() != _entryList_.size()) return;
  for( size_t iEntry = 0 ; iEntry < _entryList_.size() ; iEntry++ ){
    _entryList_[iEntry].description = descriptionList_[iEntry];
  }
}
void Selector::clearTags(){
  this->invalidateCache();
  for( auto& entry : _entryList_ ){ entry.tag = ""; }
}
void Selector::clearDescriptions(){
  this->invalidateCache();
  for( auto& entry : _entryList_ ){ entry.description.clear(); }
}

// native getters
size_t Selector::getCursorPosition() const {
  return _cursorPosition_;
}
const std::vector<SelectorEntry> &Selector::getEntryList() const {
  return _entryList_;
}
MenuLineList &Selector::getHeader() {
  return _header_;
}
MenuLineList &Selector::getFooter() {
  return _footer_;
}


// non native getters
const SelectorEntry& Selector::getSelectedEntry() const {
  if( _entryList_.empty() ){ return _dummyEntry_; }
  return _entryList_[_cursorPosition_];
}
const std::string& Selector::getSelectedEntryTitle() const {
  return this->getSelectedEntry().title;
}
size_t Selector::getNbMenuLines() const{
  return _header_.lineList.size() + _footer_.lineList.size();
}
size_t Selector::getCursorPage() const{
  this->refillPageEntryCache();

  for( size_t iPage = 0 ; iPage < _pageEntryCache_.size() ; iPage++ ){
    if( GenericToolbox::doesElementIsInVector( _cursorPosition_, _pageEntryCache_[iPage] ) ){
      return iPage;
    }
  }

  // default if no entry is defined
  return 0;
}
size_t Selector::getNbPages() const {
  this->refillPageEntryCache();
  return _pageEntryCache_.size();
}
bool Selector::isSelectedEntry(const SelectorEntry& entry_) const{
  return &entry_ == &(this->getSelectedEntry());
}

// io
void Selector::printTerminal() const {
  // clear console
  consoleClear();

  Selector::printMenu( _header_ );

  // get the paging wrt headers/footers
  this->refillPageEntryCache();

  // fetch the current page to print using the cursor position
  size_t currentPage{ this->getCursorPage()  };

  // print only the entries that
  std::stringstream ssLeft;
  for( auto& entryIndex : _pageEntryCache_[currentPage] ){

    ssLeft.str("");

    if( entryIndex == _cursorPosition_ ){ ssLeft << _cursorMarker_; }
    else{ ssLeft << " "; }
    ssLeft << " " << _entryList_[entryIndex].title;

    Terminal::printLeftRight(
        ssLeft.str(), _entryList_[entryIndex].tag,
        (entryIndex == _cursorPosition_ ? GenericToolbox::ColorCodes::blueBackground : "")
    );

    for( auto& descriptionLine : _entryList_[entryIndex].description ){
      Terminal::printLeft(
          descriptionLine,
          (entryIndex == _cursorPosition_ ? GenericToolbox::ColorCodes::blueBackground : "")
      );
    }

  }

  Selector::printMenu( _footer_ );

  consoleUpdate(nullptr);
}
void Selector::scanInputs( u64 kDown, u64 kHeld ){

  // manage persistence
  if( kHeld != 0 and kHeld == _previousKheld_ ){ _holdingTiks_++; }
  else{ _holdingTiks_ = 0; }

  // cache last key
  _previousKheld_ = kHeld;

  if( kDown == 0 and kHeld == 0 ){ return; }

  if( kDown & HidNpadButton_AnyDown or
      ( kHeld & HidNpadButton_AnyDown and _holdingTiks_ > 15 and _holdingTiks_ % 3 == 0 )
  ){
    this->selectNextEntry();
  }
  else if( kDown & HidNpadButton_AnyUp or
      ( kHeld & HidNpadButton_AnyUp and _holdingTiks_ > 15 and _holdingTiks_ % 3 == 0 )
  ){
    this->selectPrevious();
  }
  else if( kDown & HidNpadButton_AnyLeft or
    ( kHeld & HidNpadButton_AnyLeft and _holdingTiks_ > 15 and _holdingTiks_ % 3 == 0 )
  ){
    this->jumpToPreviousPage();
  }
  else if( kDown & HidNpadButton_AnyRight or
    ( kHeld & HidNpadButton_AnyRight and _holdingTiks_ > 15 and _holdingTiks_ % 3 == 0 )
  ){
    this->jumpToNextPage();
  }

}
void Selector::clearMenu(){
  _header_.clear();
  _footer_.clear();
}

// move cursor
void Selector::moveCursorPosition(long cursorPosition_){
  // if no entry, stay at 0
  if( _entryList_.empty() ){ _cursorPosition_ = 0; return; }

  // modulus to a valid index
  cursorPosition_ %= long( _entryList_.size() );
  if( cursorPosition_ < 0 ){ cursorPosition_ += long( _entryList_.size() ); }

  // set the valid cursor position
  _cursorPosition_ = cursorPosition_;
}
void Selector::jumpToPage(long pageIndex_){
  this->refillPageEntryCache();

  // if no page is set, don't do anything
  if( _pageEntryCache_.empty() ){ return; }

  // make sure the page index is valid
  pageIndex_ %= long( _pageEntryCache_.size() );
  if( pageIndex_ < 0 ){ pageIndex_ += long( _pageEntryCache_.size() ); }

  // if no entry in this page, don't do anything
  if( _pageEntryCache_[pageIndex_].empty() ){ return; }

  this->moveCursorPosition( long( _pageEntryCache_[pageIndex_][0] ) );
}
void Selector::selectNextEntry(){
  this->moveCursorPosition(long(_cursorPosition_) + 1);
}
void Selector::selectPrevious(){
  this->moveCursorPosition(long(_cursorPosition_) - 1);
}
void Selector::jumpToNextPage(){
  this->jumpToPage( long( this->getCursorPage() ) + 1 );
}
void Selector::jumpToPreviousPage(){
  this->jumpToPage( long( this->getCursorPage() ) - 1 );
}

// static
void Selector::printMenu(const MenuLineList& menu_){
  for( auto& menuLine : menu_.lineList ){
    if(not menuLine.leftPrint.str().empty() and not menuLine.rightPrint.str().empty() ){
      Terminal::printLeftRight(menuLine.leftPrint.str(), menuLine.rightPrint.str() );
    }
    else if( not menuLine.leftPrint.str().empty() ){
      Terminal::printLeft(menuLine.leftPrint.str() );
    }
    else if( not menuLine.rightPrint.str().empty() ){
      Terminal::printRight(menuLine.rightPrint.str() );
    }
  }
}
std::string Selector::askQuestion(const std::string& question_, const std::vector<std::string>& answers_,
                                  const std::vector<std::vector<std::string>>& descriptions_ ) {

  std::string answer;
  Selector sel;

  // set the layout with the question:
  sel.getHeader() >> "SimpleModManager v" >> Toolbox::getAppVersion() << std::endl;
  sel.getHeader() << GenericToolbox::repeatString("*", Hardware::getTerminalWidth()) << std::endl;
  sel.getHeader() << question_ << std::endl;
  sel.getHeader() << GenericToolbox::repeatString("*", Hardware::getTerminalWidth());

  // possible answers
  sel.setEntryList( answers_ );

  // fill optional description of the selectable items
  if( descriptions_.size() == answers_.size() ){
    sel.setDescriptionList( descriptions_ );
  }

  // footer
  sel.getFooter() << GenericToolbox::repeatString("*", Hardware::getTerminalWidth()) << std::endl;
  sel.getFooter() << " A: Select" >> "B: back";



  u64 kDown = 1;
  while(appletMainLoop()){

    // printout
    if( kDown != 0 ) { sel.printTerminal(); }

    //Scan all the inputs. This should be done once for each frame
    padUpdate( &GlobalObjects::gPad );

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);

    if     (kDown & HidNpadButton_AnyDown){
      sel.selectNextEntry();
    }
    else if(kDown & HidNpadButton_AnyUp){
      sel.selectPrevious();
    }
    else if(kDown & HidNpadButton_A){
      answer = sel.getSelectedEntryTitle();
      break;
    }
    else if(kDown & HidNpadButton_B){
      break;
    }

  }
  consoleClear();
  return answer;
}

// protected
void Selector::invalidateCache() const{
  _isPageEntryCacheValid_ = false;
}
void Selector::refillPageEntryCache() const {
  if( _isPageEntryCacheValid_ ) return;

  // reset the cache
  _pageEntryCache_.clear();
  _pageEntryCache_.emplace_back();

  long nLinesLeft{long(this->getNbMenuLines())};
  for( size_t iEntry = 0 ; iEntry < _entryList_.size() ; iEntry++ ){

    // count how many lines would be left if the entry got printed
    nLinesLeft -= long( _entryList_[iEntry].getNbPrintLines() );

    if( _pageEntryCache_.back().empty() ){
      // add the entry even if it's too long
    }
    else if( nLinesLeft < 0 ){
      // next page and reset counter
      _pageEntryCache_.emplace_back();
      nLinesLeft = long(this->getNbMenuLines());

      // it's going to be printed on this new page. Count for it
      nLinesLeft -= long( _entryList_[iEntry].getNbPrintLines() );
    }

    _pageEntryCache_.back().emplace_back( iEntry );
  }

  _isPageEntryCacheValid_ = true;
}

