//
// Created by Nadrino on 13/02/2020.
//

#ifndef SIMPLEMODMANAGER_MODSPRESETHANDLER_H
#define SIMPLEMODMANAGER_MODSPRESETHANDLER_H

#include "Selector.h"

#include <string>
#include <vector>
#include "map"


struct PresetData {
  std::string name{};
  std::vector<std::string> modList{};
};

class ModsPresetHandler {

public:

  ModsPresetHandler() = default;

  void setModFolder(const std::string &gameFolder_);

  [[nodiscard]] const std::vector<PresetData> &getPresetList() const;

  Selector &getSelector();

  void selectModPreset();
  void createNewPreset();
  void deleteSelectedPreset();
  void editPreset( size_t entryIndex_ );
  void showConflictingFiles( size_t entryIndex_ );

  [[nodiscard]] std::vector<std::string> generatePresetNameList() const;

  // non native getters
  std::string getSelectedModPresetName() const;
  [[nodiscard]] const std::vector<std::string>& getSelectedPresetModList() const;


protected:
  void readConfigFile();
  void writeConfigFile();
  void fillSelector();

private:
  std::string _gameFolder_{};

  std::vector<PresetData> _presetList_{};

//  std::vector<std::string> _presets_list_;
//  std::map<std::string, std::vector<std::string>> _dataHandler_;

  Selector _selector_;

};




#endif //SIMPLEMODMANAGER_MODSPRESETHANDLER_H
