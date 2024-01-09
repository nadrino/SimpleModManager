//
// Created by Nadrino on 16/10/2019.
//

#include "ConfigHandler.h"
#include "Toolbox.h"

#include "GenericToolbox.Switch.h"
#include "GenericToolbox.Vector.h"

#include <iostream>

// struct
void ConfigHolder::setSelectedPresetIndex(int selectedPresetIndex_){
  if( presetList.empty() ){ selectedPresetIndex = -1; return; }
  selectedPresetIndex_ %= int( presetList.size() );
  if( selectedPresetIndex_ < 0 ) selectedPresetIndex_ += int( presetList.size() );
  selectedPresetIndex = selectedPresetIndex_;
}
void ConfigHolder::setSelectedPreset(const std::string& preset_){
  setSelectedPresetIndex( GenericToolbox::findElementIndex(preset_, presetList, [](const PresetConfig& entry_){ return entry_.name; }) );
}
std::string ConfigHolder::getCurrentPresetName() const{
  if( presetList.empty() or selectedPresetIndex >= int( presetList.size() ) ){ return {}; }
  return presetList[selectedPresetIndex].name;
}

// io
void ConfigHandler::loadConfig(const std::string &configFilePath_) {
  ConfigHolder config{}; // loads defaults
  config.presetList.clear(); // reset the default presets
  config.configFilePath = configFilePath_;

  if( config.configFilePath.empty() ){
    // default path or reload
    config.configFilePath = _config_.configFilePath;
  }

  std::string lastUsedPresetName{"default"};

  if( not GenericToolbox::isFile(config.configFilePath) ){
    // immediately dump the default config to the file
    this->dumpConfigToFile();
    return;
  }

  // parse the config file
  auto configLines = GenericToolbox::dumpFileAsVectorString( config.configFilePath, true );
  for( auto& line : configLines ){

    // removing heading and trailing spaces
    GenericToolbox::trimInputString(line, " ");

    // check if it is a comment
    if( GenericToolbox::startsWith(line, "#") ) continue;

    // check if it is a valid piece of data
    auto elements = GenericToolbox::splitString(line, "=");
    if( elements.size() != 2 ) continue;

    // removing heading and trailing spaces
    GenericToolbox::trimInputString( elements[0], " " );
    GenericToolbox::trimInputString( elements[1], " " );

    if     ( elements[0] == "use-gui" ){
      config.useGui = GenericToolbox::toBool( elements[1] );
    }
    else if( elements[0] == "sort-game-list-by" ){
      config.sortGameList = SortGameList::toEnum( elements[1] );
    }
    else if( elements[0] == "stored-mods-base-folder" ){
      config.baseFolder = elements[1];
    }
    else if( elements[0] == "last-preset-used" ){
      lastUsedPresetName = elements[1];
    }
    else if( elements[0] == "last-program-version" ){
      config.lastSmmVersion = elements[1];
    }
    else if( elements[0] == "preset" ){
      config.presetList.emplace_back();
      config.presetList.back().name = elements[1];
    }
    else if( elements[0] == "install-mods-base-folder" ){
      if( config.presetList.empty() ){
        config.presetList.emplace_back();
        config.presetList.back().name = "default";
      }
      config.presetList.back().installBaseFolder = elements[1];
    }

  } // lines

  // look for the selected preset index. If not found, will stay at 0
  this->selectPresetWithName( lastUsedPresetName );

  // copy to the member
  _config_ = config;

  // rewrite for cleanup
  this->dumpConfigToFile();
}
void ConfigHandler::dumpConfigToFile() const {

  GenericToolbox::mkdir( GenericToolbox::getFolderPath( _config_.configFilePath ) );

  std::stringstream ssConfig;
  ssConfig << "# This is a config file" << std::endl;
  ssConfig << std::endl;
  ssConfig << "# folder where mods are stored" << std::endl;
  ssConfig << "stored-mods-base-folder = " << _config_.baseFolder << std::endl;
  ssConfig << "use-gui = " << _config_.useGui << std::endl;
  ssConfig << "sort-game-list-by = " << _config_.sortGameList.toString() << std::endl;
  ssConfig << "last-preset-used = " << _config_.getCurrentPresetName() << std::endl;
  ssConfig << std::endl;
  ssConfig << std::endl;
  for( auto &preset : _config_.presetList ){
    ssConfig << "########################################" << std::endl;
    ssConfig << "# preset that can be changed in the app" << std::endl;
    ssConfig << "preset = " << preset.name << std::endl;
    ssConfig << std::endl;
    ssConfig << "# base folder where mods are installed" << std::endl;
    ssConfig << "install-mods-base-folder = " << preset.installBaseFolder << std::endl;
    ssConfig << "########################################" << std::endl;
    ssConfig << std::endl;
    ssConfig << std::endl;
  }
  ssConfig << "# DO NOT TOUCH THIS : used to recognise the last version of the program config" << std::endl;
  ssConfig << "last-program-version = " << Toolbox::getAppVersion() << std::endl;
  ssConfig << std::endl;

  GenericToolbox::dumpStringInFile(_config_.configFilePath, ssConfig.str());
}

// preset selection
void ConfigHandler::setCurrentConfigPresetId(int selectedPresetId_){
  _config_.setSelectedPresetIndex( selectedPresetId_ );
  // auto save last-preset-used
  this->dumpConfigToFile();
}
void ConfigHandler::selectPresetWithName(const std::string &presetName_){
  for( size_t iPreset = 0 ; iPreset < _config_.presetList.size() ; iPreset++ ){
    if( _config_.presetList[iPreset].name == presetName_ ){
      this->setCurrentConfigPresetId( int( iPreset ) );
      return;
    }
  }
}
void ConfigHandler::selectNextPreset(){
  _config_.setSelectedPresetIndex( _config_.selectedPresetIndex + 1 );
}
void ConfigHandler::selectPreviousPreset(){
  _config_.setSelectedPresetIndex( _config_.selectedPresetIndex - 1 );
}




