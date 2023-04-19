//
// Created by Nadrino on 13/02/2020.
//

#include <Toolbox.h>
#include <ModsPresetHandler.h>
#include <Selector.h>
#include "GlobalObjects.h"

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include "sstream"


void ModsPresetHandler::setModFolder(const std::string &modFolder) {
  _modFolder_ = modFolder;
  this->readConfigFile();
}

const std::vector<PresetData> &ModsPresetHandler::getPresetList() const {
  return _presetList_;
}

Selector &ModsPresetHandler::getSelector() {
  return _selector_;
}

void ModsPresetHandler::selectModPreset() {

  auto drawSelectorPage = [&]{
    consoleClear();
    GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
    std::cout << GenericToolbox::ColorCodes::redBackground << std::setw(GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::left;
    std::cout << "Select mod preset" << GenericToolbox::ColorCodes::resetColor;
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    _selector_.print();
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeft("  Page (" + std::to_string(_selector_.getCursorPage() + 1) + "/" + std::to_string(
        _selector_.getNbPages()) + ")");
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeftRight(" A : Select mod preset", " X : Delete mod preset ");
    GenericToolbox::Switch::Terminal::printLeftRight(" Y : Edit preset", "+ : Create a preset ");
    GenericToolbox::Switch::Terminal::printLeft(" B : Go back");
    consoleUpdate(nullptr);
  };

  drawSelectorPage();
  while(appletMainLoop()){

    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    u64 kHeld = padGetButtons(&GlobalObjects::gPad);
    _selector_.scanInputs(kDown, kHeld);
    if(kDown & HidNpadButton_B){
      break;
    }
    else if( kDown & HidNpadButton_A and not _presetList_.empty() ){
      return;
    }
    else if(kDown & HidNpadButton_X and not _presetList_.empty()){
      std::string answer = Selector::ask_question(
        "Are you sure you want to remove this preset ?",
        std::vector<std::string>({"Yes", "No"})
      );
      if( answer == "Yes" ) this->deleteSelectedPreset();
    }
    else if(kDown & HidNpadButton_Plus){ createNewPreset(); }
    else if(kDown & HidNpadButton_Y){ this->editPreset( _selector_.getCursorPage() ); }

    if( kDown != 0 or kHeld != 0 ){ drawSelectorPage(); }

  }
}
void ModsPresetHandler::createNewPreset(){
  // generate a new default name
  int iPreset{1};
  std::stringstream ss;
  auto presetNameList = this->generatePresetNameList();
  do{ ss.str(""); ss << "preset-" << iPreset++; }
  while( GenericToolbox::doesElementIsInVector( ss.str(), presetNameList ) );

  // create a new entry
  size_t presetIndex{_presetList_.size()};
  _presetList_.emplace_back();
  _presetList_.back().name = ss.str();

  // start the editor
  this->editPreset( presetIndex );
}
void ModsPresetHandler::deleteSelectedPreset(){
  _presetList_.erase( _presetList_.begin() + _selector_.getSelectedEntryIndex() );
  this->writeConfigFile();
  this->readConfigFile();
}
void ModsPresetHandler::editPreset( size_t entryIndex_ ) {

  auto& preset = _presetList_[entryIndex_];

  std::vector<std::string> modsList = GenericToolbox::getListOfFilesInSubFolders(_modFolder_);
  std::sort( modsList.begin(), modsList.end() );
  Selector sel;
  sel.setEntryList(modsList);

  auto reprocessSelectorTags = [&]{
    // clear tags
    sel.clearTags();

    for(size_t presetModIndex = 0 ; presetModIndex < preset.modList.size() ; presetModIndex++ ){
      for( size_t jEntry = 0 ; jEntry < modsList.size() ; jEntry++ ){

        if(preset.modList[presetModIndex] == modsList[jEntry] ){
          // add selector tag to the given mod
          std::stringstream ss;
          ss << sel.getTag( jEntry );
          if( not ss.str().empty() ) ss << " & ";
          ss << "#" << presetModIndex;
          sel.setTag( jEntry, ss.str() );
          break;
        }

      }
    }
  };


  auto printSelector = [&]{
    consoleClear();
    GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
    std::cout << GenericToolbox::ColorCodes::redBackground << std::setw(GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::left;
    std::string header_title = "Creating preset : " + preset.name + ". Select the mods you want.";
    std::cout << header_title << GenericToolbox::ColorCodes::resetColor;
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    sel.print();
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeftRight(" A : Add mod", "X : Cancel mod ");
    GenericToolbox::Switch::Terminal::printLeftRight(" + : SAVE", "B : Abort / Go back ");
    consoleUpdate(nullptr);
  };


  printSelector();
  while(appletMainLoop()){

    padUpdate(&GlobalObjects::gPad);;
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    u64 kHeld = padGetButtons(&GlobalObjects::gPad);
    sel.scanInputs(kDown, kHeld);
    if(kDown & HidNpadButton_A){
      preset.modList.emplace_back(sel.getSelectedEntryTitle() );
      reprocessSelectorTags();
    }
    else if(kDown & HidNpadButton_X){
      preset.modList.pop_back();
      reprocessSelectorTags();
    }
    else if( kDown & HidNpadButton_Plus ){
      // save changes
      break;
    }
    else if( kDown & HidNpadButton_B ){
      // discard changes
      return;
    }

    if( kDown != 0 or kHeld != 0 ){ printSelector(); }
  }


  preset.name = GenericToolbox::Switch::UI::openKeyboardUi( preset.name );

  // Check for conflicts
  this->showConflictingFiles( entryIndex_ );
  this->writeConfigFile();
  this->readConfigFile();
}
void ModsPresetHandler::showConflictingFiles( size_t entryIndex_ ) {
  auto& preset = _presetList_[entryIndex_];

  consoleClear();

  GenericToolbox::Switch::Terminal::printLeft("Scanning preset files...", GenericToolbox::ColorCodes::magentaBackground);
  consoleUpdate(nullptr);

  struct ModFileEntry{
    long finalFileSize{0};
    std::vector<std::string> fromModList{};
  };

  std::map<std::string, ModFileEntry> installedFileList;

  for( auto& mod : preset.modList ){
    GenericToolbox::Switch::Terminal::printLeft(" > Getting files for the mod: " + mod, GenericToolbox::ColorCodes::magentaBackground);
    consoleUpdate(nullptr);

    auto filesList = GenericToolbox::getListOfFilesInSubFolders( _modFolder_ + "/" + mod );
    for( auto& file: filesList ){
      std::stringstream ss;
      ss << _modFolder_ << "/" << mod << "/" << file;
      installedFileList[file].finalFileSize = GenericToolbox::getFileSize( ss.str() );
      installedFileList[file].fromModList.emplace_back( mod );
    }
  }

  double presetSize{0};
  for( auto& installedFile : installedFileList ){
    presetSize += double( installedFile.second.finalFileSize );
  }

//  std::string total_SD_size_str = GenericToolbox::parseSizeUnits(total_SD_size);

  std::vector<std::string> conflictFileList;
  std::vector<std::string> overridingModList;
  for( auto& installedFile : installedFileList ){
    if( installedFile.second.fromModList.size() == 1 ){ continue; }
    conflictFileList.emplace_back( installedFile.first );
    overridingModList.emplace_back( installedFile.second.fromModList.back() );
  }

  Selector sel;
  sel.setEntryList(conflictFileList);
  sel.setTagList(overridingModList);
  sel.setMaxItemsPerPage(GenericToolbox::Switch::Hardware::getTerminalHeight() - 9);

  auto printSelector = [&]{
    consoleClear();
    GenericToolbox::Switch::Terminal::printRight("SimpleModManager v" + Toolbox::get_app_version());
    GenericToolbox::Switch::Terminal::printLeft("Conflicted files for the preset \"" + preset.name + "\":", GenericToolbox::ColorCodes::redBackground);
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    sel.print();
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeft("Total size of the preset:" + GenericToolbox::parseSizeUnits(presetSize), GenericToolbox::ColorCodes::greenBackground);
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeft("Page (" + std::to_string(sel.getCurrentPage() + 1) + "/" + std::to_string(
        sel.getNbPages()) + ")");
    std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
    GenericToolbox::Switch::Terminal::printLeft(" A : OK");
    if(sel.getNbPages() > 1) GenericToolbox::Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
    consoleUpdate(nullptr);
  };

  // Main loop
  u64 kDown{0}, kHeld{0};
  printSelector();
  while(appletMainLoop())
  {

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);
    kHeld = padGetButtons(&GlobalObjects::gPad);

    if (kDown & HidNpadButton_A) {
      break; // break in order to return to hbmenu
    }

    sel.scanInputs(kDown, kHeld);
    if( kDown != 0 or kHeld != 0 ){ printSelector(); }
  }

}

std::vector<std::string> ModsPresetHandler::generatePresetNameList() const{
  std::vector<std::string> out;
  out.reserve(_presetList_.size());
  for( auto& preset : _presetList_ ){
    out.emplace_back( preset.name );
  }
  return out;
}

const std::string& ModsPresetHandler::getSelectedModPresetName() const {
  return _presetList_[_selector_.getSelectedEntryIndex()].name;
}
const std::vector<std::string>& ModsPresetHandler::getSelectedPresetModList() const{
  return _presetList_[_selector_.getSelectedEntryIndex()].modList;
}

void ModsPresetHandler::readConfigFile() {
  _presetList_.clear();

  // check if file exist
  auto lines = GenericToolbox::dumpFileAsVectorString( _modFolder_ + "/mod_presets.conf", true );

  for( auto &line : lines ){
    if(line[0] == '#') continue;

    auto lineElements = GenericToolbox::splitString(line, "=", true);
    if( lineElements.size() != 2 ) continue;

    // clean up for extra spaces characters
    for(auto &element : lineElements){ GenericToolbox::trimString(element, " "); }

    if( lineElements[0] == "preset" ){
      _presetList_.emplace_back();
      _presetList_.back().name = lineElements[1];
    }
    else {
      // element 0 is "mod7" for example. Irrelevant here
      _presetList_.back().modList.emplace_back(lineElements[1] );
    }
  }

  this->fillSelector();
}
void ModsPresetHandler::writeConfigFile() {

  std::stringstream ss;

  ss << "# This is a config file" << std::endl;
  ss << std::endl;
  ss << std::endl;

  for(auto const &preset : _presetList_ ){
    ss << "########################################" << std::endl;
    ss << "# mods preset name" << std::endl;
    ss << "preset = " << preset.name << std::endl;
    ss << std::endl;
    ss << "# mods list" << std::endl;
    int iMod{0};
    for( auto& mod : preset.modList ){
      ss << "mod" << iMod++ << " = " << mod << std::endl;
    }
    ss << "########################################" << std::endl;
    ss << std::endl;
  }

  std::string data = ss.str();
  GenericToolbox::dumpStringInFile(_modFolder_ + "/mod_presets.conf", data);

}
void ModsPresetHandler::fillSelector(){
  _selector_.reset();

  if( _presetList_.empty() ){
    _selector_.setEntryList({{"NO MODS PRESETS"}});
    return;
  }

  auto presetNameList = this->generatePresetNameList();
  std::vector<std::vector<std::string>> descriptionsList;
  descriptionsList.reserve( presetNameList.size() );
  for( auto& preset: _presetList_ ){
    descriptionsList.emplace_back();
    descriptionsList.back().reserve( preset.modList.size() );
    for( auto& mod : preset.modList ){ descriptionsList.back().emplace_back("  | " + mod ); }
  }

  _selector_.setEntryList(presetNameList);
  _selector_.setDescriptionList(descriptionsList);
}





