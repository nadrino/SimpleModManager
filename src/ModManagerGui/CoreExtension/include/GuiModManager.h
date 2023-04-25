//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_GUIMODMANAGER_H
#define SIMPLEMODMANAGER_GUIMODMANAGER_H

#include <PopupLoadingView.h>
#include <PopupLoadingBox.h>

#include "GameBrowser.h"

#include <borealis.hpp>

#include <string>
#include <future>


class GuiModManager {

public:
  GuiModManager() = default;

  void setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus);

  [[nodiscard]] bool isTriggerUpdateModsDisplayedStatus() const;

  const GameBrowser &getGameBrowser() const;
  GameBrowser &getGameBrowser();

  void startApplyModThread(const std::string& modName_);
  void startRemoveModThread(const std::string& modName_);
  void startCheckAllModsThread();
  void startRemoveAllModsThread();
  void startApplyModPresetThread(const std::string &modPresetName_);

  void applyMod(const std::string &modName_, bool force_= false);
  void applyModsList(std::vector<std::string>& modsList_);
  void removeMod(const std::string &modName_);
  void removeAllMods(bool force_ = false);
  void checkAllMods();
  std::string getModStatus(const std::string &modName_, bool useCache_ = false);

protected:
  bool applyModFunction(const std::string& modName_);
  bool applyModPresetFunction(const std::string& presetName_);
  bool removeModFunction(const std::string& modName_);
  bool checkAllModsFunction();
  bool removeAllModsFunction();


private:
  // core
  GameBrowser _gameBrowser_{};

  PopupLoadingBox _loadingBox_{};
  std::future<bool> _asyncResponse_{};

  bool _triggerUpdateModsDisplayedStatus_{false};

};


#endif //SIMPLEMODMANAGER_GUIMODMANAGER_H
