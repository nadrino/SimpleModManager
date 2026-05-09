//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_GUIMODMANAGER_H
#define SIMPLEMODMANAGER_GUIMODMANAGER_H


#include "GameBrowser.h"
#include "GenericToolbox.Borealis.h"

#include <borealis.hpp>

#include <string>
#include <future>
#include <atomic>


class GuiModManager {

public:
  GuiModManager() = default;

  // setters
  void setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus);
  void setTriggerRebuildModBrowser(bool triggerRebuildModBrowser);

  // getters
  [[nodiscard]] bool isTriggerUpdateModsDisplayedStatus() const;
  [[nodiscard]] bool isTriggerRebuildModBrowser() const;
  const GameBrowser &getGameBrowser() const;
  GameBrowser &getGameBrowser();

  void startApplyModThread(const std::string& modName_);
  void startRemoveModThread(const std::string& modName_);
  bool startDeleteModFolderThread(const std::string& modName_);
  bool startDeleteOrphanInstalledModsThread(const std::vector<std::string>& modNameList_);
  void startCheckAllModsThread();
  void startRemoveAllModsThread();
  void startApplyModPresetThread(const std::string &modPresetName_);
  bool isBackgroundTaskRunning() const;
  bool canStartDeleteModFolderThread() const;

  void applyMod(const std::string &modName_);
  void applyModsList(std::vector<std::string>& modsList_);
  void removeMod(const std::string &modName_);
  bool deleteModFolderFromSd(const std::string &modName_);
  bool deleteOrphanInstalledMods(const std::vector<std::string>& modNameList_);
  void removeAllMods();
  void checkAllMods(bool useCache_ = false);
  void getModStatus(const std::string &modName_, bool useCache_ = false);

protected:
  bool applyModFunction(const std::string& modName_);
  bool applyModPresetFunction(const std::string& presetName_);
  bool removeModFunction(const std::string& modName_);
  bool deleteModFolderFunction(const std::string& modName_);
  bool deleteOrphanInstalledModsFunction(std::vector<std::string> modNameList_);
  bool checkAllModsFunction();
  bool removeAllModsFunction();

  void removeModInstalledFiles(const std::string &modName_, bool forceUnknownInstalledFiles_);
  bool cleanupInstalledFilesByRelativePaths(const std::vector<std::string>& relativePathList_, const std::string& label_, bool allowCurrentSdOwnedFiles_);
  bool deleteOrphanInstalledModsPass(const std::vector<std::string>& modNameList_, bool forceDelete_);
  bool leaveModAction(bool isSuccess_);
  void finishDeleteModFolderTask();



private:
  // core
  GameBrowser _gameBrowser_{};

  std::future<bool> _asyncResponse_{};

  bool _triggeredOnCancel_{false};
  bool _triggerUpdateModsDisplayedStatus_{false};
  bool _triggerRebuildModBrowser_{false};
  std::atomic<bool> _deleteModFolderRunning_{false};
  std::atomic<long long> _lastDeleteModFolderFinishedMs_{0};


  // monitors
  GenericToolbox::Borealis::PopupLoadingBox _loadingPopup_;
  struct ModApplyMonitor{
    double progress{0};
    std::string currentFile{};
  }; ModApplyMonitor modApplyMonitor{};

  struct ModApplyListMonitor{
    double progress{0};
    std::string currentMod{};
  }; ModApplyListMonitor modApplyListMonitor{};
  struct ModCheckMonitor{
    double progress{0};
    std::string currentFile{};
  }; ModCheckMonitor modCheckMonitor{};

  struct ModCheckAllMonitor{
    double progress{0};
    std::string currentMod{};
  }; ModCheckAllMonitor modCheckAllMonitor{};

  struct ModRemoveMonitor{
    double progress{0};
    std::string currentFile{};
  }; ModRemoveMonitor modRemoveMonitor{};

  struct ModRemoveAllMonitor{
    double progress{0};
    std::string currentMod{};
  }; ModRemoveAllMonitor modRemoveAllMonitor{};

  struct ModDeleteFolderMonitor{
    double progress{0};
    std::string currentEntry{};
  }; ModDeleteFolderMonitor modDeleteFolderMonitor{};

};


#endif //SIMPLEMODMANAGER_GUIMODMANAGER_H
