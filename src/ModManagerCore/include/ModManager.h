//
// Created by Nadrino on 06/09/2019.
//

#ifndef MODAPPLIER_MOD_MANAGER_H
#define MODAPPLIER_MOD_MANAGER_H

#include <ConfigHandler.h>

#include <utility>
#include <vector>
#include <string>
#include <map>


struct ModEntry{
  ModEntry() = default;
  explicit ModEntry(std::string  modName_): modName(std::move(modName_)) {}

  std::string modName;
  std::string statusStr{"UNCHECKED"};
  double applyFraction{0};
};


class ModBrowser;

class ModManager {

public:
  explicit ModManager(ModBrowser* owner_);

  // setters
  void setGameFolderPath(const std::string &gameFolderPath_);
  void setIgnoredFileList(std::vector<std::string>& ignoredFileList_);

  // getters
  [[nodiscard]] const std::string & getCurrentModFolderPath() const;
  [[nodiscard]] const std::vector<std::string> & getIgnoredFileList() const;

  void updateModList();
  void dumpModStatusCache();
  void resetModCache(const std::string &modName_);
  void resetAllModsCacheAndFile();

  void updateModStatus(std::string& modName_);

  double getModApplyFraction(const std::string &modName_);
  std::string generateStatusStr(const std::string& modName_);

  void applyMod(const std::string& modName_, bool overrideConflicts_ = false);
  void applyModList(const std::vector<std::string> &modNamesList_);
  void removeMod(const std::string &modName_);

  void displayModFilesStatus(const std::string &modName_);

  ModEntry* fetchModEntry(const std::string& modName_);


private:
  ModBrowser* _owner_{nullptr};

  bool _ignoreCacheFiles_{true};
  std::string _gameFolderPath_{};
  std::vector<std::string> _ignoredFileList_{};
//  std::map<std::string, std::vector<std::string>> _relative_file_path_list_cache_;

  std::vector<ModEntry> _modList_{};
  static ModEntry dummyModEntry;

};


#endif //MODAPPLIER_MOD_MANAGER_H
