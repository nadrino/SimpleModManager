//
// Created by Nadrino on 06/09/2019.
//

#ifndef MODAPPLIER_MOD_MANAGER_H
#define MODAPPLIER_MOD_MANAGER_H

#include <ConfigHandler.h>
#include "Selector.h"

#include <utility>
#include <vector>
#include <string>
#include <map>

struct ApplyCache{
  std::string statusStr{"UNCHECKED"};
  double applyFraction{0};
};

struct ModEntry{
  ModEntry() = default;
  explicit ModEntry(std::string  modName_): modName(std::move(modName_)) {}

  std::string modName;
  std::map<std::string, ApplyCache> applyCache;
};

ENUM_EXPANDER(
    ResultModAction, 0,
    Success,
    Fail,
    Abort
);


class GameBrowser;

class ModManager {

public:
  explicit ModManager(GameBrowser* owner_);

  // setters
  void setAllowAbort(bool allowAbort);
  void setGameFolderPath(const std::string &gameFolderPath_);
  void setIgnoredFileList(std::vector<std::string>& ignoredFileList_);

  // getters
  const Selector &getSelector() const;
  const std::vector<ModEntry> &getModList() const;
  [[nodiscard]] const std::string & getGameFolderPath() const;
  [[nodiscard]] const std::vector<std::string> & getIgnoredFileList() const;

  std::vector<ModEntry> &getModList();
  std::vector<std::string> & getIgnoredFileList();

  // shortcuts
  const ConfigHolder& getConfig() const;
  ConfigHolder& getConfig();

  // selector related
  void updateModList();
  void dumpModStatusCache();
  void reloadModStatusCache();
  void resetAllModsCacheAndFile();

  // mod management
  void resetModCache(int modIndex_);
  void resetModCache(const std::string &modName_);

  ResultModAction updateModStatus(int modIndex_);
  ResultModAction updateModStatus(const std::string& modName_);
  ResultModAction updateAllModStatus();

  ResultModAction applyMod(int modIndex_, bool overrideConflicts_ = false);
  ResultModAction applyMod(const std::string& modName_, bool overrideConflicts_ = false);
  ResultModAction applyModList(const std::vector<std::string> &modNamesList_);

  void removeMod(int modIndex_);
  void removeMod(const std::string &modName_);



  // terminal
  void scanInputs(u64 kDown, u64 kHeld);
  void printTerminal();
  void rebuildSelectorMenu();
  void displayModFilesStatus(const std::string &modName_);

  // utils
  int getModIndex(const std::string& modName_);

protected:
  void displayConflictsWithOtherMods(size_t modIndex_);

private:
  GameBrowser* _owner_{nullptr};

  bool _ignoreCacheFiles_{true};
  bool _allowAbort_{true};
  std::string _gameFolderPath_{};
  std::vector<std::string> _ignoredFileList_{};

  Selector _selector_;
  std::vector<ModEntry> _modList_{};
};


#endif //MODAPPLIER_MOD_MANAGER_H
