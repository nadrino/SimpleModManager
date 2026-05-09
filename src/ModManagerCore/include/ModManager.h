//
// Created by Nadrino on 06/09/2019.
//

#ifndef MODAPPLIER_MOD_MANAGER_H
#define MODAPPLIER_MOD_MANAGER_H

#include <ConfigHandler.h>
#include "Selector.h"

#include "GenericToolbox.Map.h"

#include <map>
#include <vector>
#include <string>
#include <utility>

struct ModFileStatusCache{
  std::string state{"MISSING"};
  long long sourceSize{-1};
  long long sourceMtime{0};
  bool destinationExists{false};
  long long destinationSize{-1};
  long long destinationMtime{0};
};

struct ApplyCache{
  std::string statusStr{"UNCHECKED"};
  double applyFraction{0};
  size_t totalFiles{0};
  size_t matchingFiles{0};
  size_t differentFiles{0};
  size_t missingFiles{0};
  std::map<std::string, ModFileStatusCache> fileStatusCache;
};

struct ModEntry{
  ModEntry() = default;
  explicit ModEntry(std::string  modName_): modName(std::move(modName_)) {}

  std::string modName;
  std::map<std::string, ApplyCache> applyCache;

  [[nodiscard]] const ApplyCache* getCache(const std::string& preset_) const {
    if( not GenericToolbox::isIn(preset_, applyCache) ){
      return nullptr;
    }
    return &applyCache.at(preset_);
  }
  [[nodiscard]] std::string getStatus(const std::string& preset_) const {
    auto* cache{this->getCache(preset_)};
    if( cache == nullptr ){ return {}; }
    return cache->statusStr;
  }
  [[nodiscard]] double getStatusFraction(const std::string& preset_) const {
    auto* cache{this->getCache(preset_)};
    if( cache == nullptr ){ return 0; }
    return cache->applyFraction;
  }
};

struct ModStatusSummary{
  size_t totalMods{0};
  size_t activeMods{0};
  size_t partialMods{0};
  size_t inactiveMods{0};
  size_t noFileMods{0};
  size_t uncheckedMods{0};
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
  void setGameName(const std::string &gameName);
  void setGameFolderPath(const std::string &gameFolderPath_);
  void setIgnoredFileList(std::vector<std::string>& ignoredFileList_);

  // getters
  [[nodiscard]] const std::string &getGameName() const;
  [[nodiscard]] const std::string &getGameFolderPath() const;
  const Selector &getSelector() const;
  [[nodiscard]] const std::vector<std::string> & getIgnoredFileList() const;
  const std::vector<ModEntry> &getModList() const;

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
  void refreshAllModStatusCache(bool forceRecheck_ = false);

  // mod management
  void resetModCache(int modIndex_);
  void resetModCache(const std::string &modName_);

  ResultModAction refreshModStatus(int modIndex_, bool forceRecheck_ = false);
  ResultModAction refreshModStatus(const std::string& modName_, bool forceRecheck_ = false);
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

  // preset
  void reloadCustomPreset();
  void setCustomPreset(const std::string &presetName_);
  const PresetConfig& fetchCurrentPreset() const;

  const std::string &getCurrentPresetName() const;

  static ModStatusSummary readGameStatusSummary(
      const std::string& gameFolderPath_,
      const std::string& presetName_
  );
  static std::string formatGameStatusSummary(const ModStatusSummary& summary_);

protected:
  void displayConflictsWithOtherMods(size_t modIndex_);

private:
  ResultModAction updateModStatusInternal(int modIndex_, bool forceRecheck_, bool showTerminalProgress_, bool dumpCache_);

  GameBrowser* _owner_{nullptr};

  bool _ignoreCacheFiles_{true};
  bool _allowAbort_{true};
  std::string _gameFolderPath_{};
  std::string _gameName_{};
  std::vector<std::string> _ignoredFileList_{};

  Selector _selector_;
  std::vector<ModEntry> _modList_{};

  std::string _currentPresetName_{};
};


#endif //MODAPPLIER_MOD_MANAGER_H
