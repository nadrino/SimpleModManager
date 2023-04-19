//
// Created by Nadrino on 16/10/2019.
//

#ifndef SIMPLEMODMANAGER_CONFIGHANDLER_H
#define SIMPLEMODMANAGER_CONFIGHANDLER_H

#include <string>
#include <vector>
#include <map>

struct PresetConfig{
  std::string name{};
  std::string installBaseFolder{};
};

struct ConfigHolder{
  bool useGui{true};
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
  [[nodiscard]] std::string getCurrentPresetName() const;
  [[nodiscard]] const PresetConfig& getCurrentPreset() const { return presetList[selectedPresetIndex]; }
};

class ConfigHandler {

public:
  ConfigHandler(){ this->loadConfig(); }
  ~ConfigHandler(){ this->dumpConfigToFile(); }

  // getters
  [[nodiscard]] const ConfigHolder &getConfig() const;
  ConfigHolder &getConfig();

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
