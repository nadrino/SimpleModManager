//
// Created by Nadrino on 16/10/2019.
//

#ifndef SIMPLEMODMANAGER_CONFIGHANDLER_H
#define SIMPLEMODMANAGER_CONFIGHANDLER_H


#include "GenericToolbox.String.h"
#include "GenericToolbox.Macro.h"

#include <map>
#include <string>
#include <vector>
#include <sstream>



struct PresetConfig{
  std::string name{};
  std::string installBaseFolder{};
};

struct ConfigHolder{

#define ENUM_NAME SortGameList
#define ENUM_FIELDS \
  ENUM_FIELD( NbMods, 0 ) \
  ENUM_FIELD( Alphabetical ) \
  ENUM_FIELD( GameLaunched ) \
  ENUM_FIELD( ModAdded ) \
  ENUM_FIELD( PlayTime ) \
  ENUM_FIELD( LaunchCount ) \
  ENUM_FIELD( NoSort )
#include "GenericToolbox.MakeEnum.h"

#define ENUM_NAME SortGameListDirection
#define ENUM_FIELDS \
  ENUM_FIELD( Ascending, 0 ) \
  ENUM_FIELD( Descending )
#include "GenericToolbox.MakeEnum.h"

  bool useGui{true};
  bool showDebugMtpFiles{false};
  SortGameList sortGameList{SortGameList::NbMods};
  SortGameListDirection sortGameListDirection{SortGameListDirection::Ascending};
  std::string baseFolder{"/mods"};
  int selectedPresetIndex{0};
  std::vector<PresetConfig> presetList{
      {{"default"}, {"/atmosphere"}},
      {{"reinx"}, {"/reinx"}},
      {{"sxos"}, {"/sxos"}},
      {{"root"}, {"/"}},
  };
  std::string lastSmmVersion{};

  std::string configFilePath{"/config/SimpleModManager/parameters.ini"};

  void setSelectedPresetIndex(int selectedPresetIndex_);
  void setSelectedPreset(const std::string& preset_);
  [[nodiscard]] std::string getCurrentPresetName() const;
  [[nodiscard]] const PresetConfig& getCurrentPreset() const { return presetList[selectedPresetIndex]; }
  [[nodiscard]] std::string getSortGameListDisplayName() const {
    switch( sortGameList.value ) {
      case SortGameList::Alphabetical: return "Alphabetical";
      case SortGameList::NbMods: return "Number of mods";
      case SortGameList::GameLaunched: return "Game launched";
      case SortGameList::ModAdded: return "Mod added";
      case SortGameList::PlayTime: return "Play time";
      case SortGameList::LaunchCount: return "Launch count";
      case SortGameList::NoSort: return "No sort";
      default: return sortGameList.toString();
    }
  }
  [[nodiscard]] std::string getSortGameListDirectionDisplayName() const {
    switch( sortGameListDirection.value ) {
      case SortGameListDirection::Ascending: return "Ascending";
      case SortGameListDirection::Descending: return "Descending";
      default: return sortGameListDirection.toString();
    }
  }
  [[nodiscard]] std::string getSortGameListSettingDisplayName() const {
    if( sortGameList == SortGameList::NoSort ){
      return getSortGameListDisplayName();
    }
    return getSortGameListDisplayName() + " (" + getSortGameListDirectionDisplayName() + ")";
  }

  [[nodiscard]] std::string getSummary() const {
    std::stringstream ss;
    ss << GET_VAR_NAME_VALUE(useGui) << std::endl;
    ss << GET_VAR_NAME_VALUE(showDebugMtpFiles) << std::endl;
    ss << GET_VAR_NAME_VALUE(sortGameList.toString()) << std::endl;
    ss << GET_VAR_NAME_VALUE(sortGameListDirection.toString()) << std::endl;
    ss << GET_VAR_NAME_VALUE(baseFolder) << std::endl;
    ss << GET_VAR_NAME_VALUE(selectedPresetIndex) << std::endl;
    ss << GET_VAR_NAME_VALUE(lastSmmVersion) << std::endl;
    ss << GET_VAR_NAME_VALUE(configFilePath) << std::endl;
    ss << GenericToolbox::toString(presetList, [](const PresetConfig& p){ return p.name + " -> " + p.installBaseFolder; });
    return ss.str();
  }
};

class ConfigHandler {

public:
  ConfigHandler(){ this->loadConfig(); }

  // getters
  [[nodiscard]] const ConfigHolder &getConfig() const{ return _config_; }
  ConfigHolder &getConfig(){ return _config_; }

  // io
  void loadConfig(const std::string& configFilePath_ = "");
  void dumpConfigToFile() const;

  // change preset
  void setCurrentConfigPresetId(int selectedPresetId_);
  void selectPresetWithName(const std::string &presetName_);
  void selectNextPreset();
  void selectPreviousPreset();

private:
  ConfigHolder _config_{};

};


#endif //SIMPLEMODMANAGER_CONFIGHANDLER_H
