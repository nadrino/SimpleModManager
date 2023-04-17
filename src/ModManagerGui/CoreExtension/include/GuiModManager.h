//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_GUIMODMANAGER_H
#define SIMPLEMODMANAGER_GUIMODMANAGER_H

#include <PopupLoadingView.h>
#include <PopupLoadingBox.h>

#include <borealis.hpp>

#include <string>
#include <future>


class GuiModManager {

public:
  static void applyMod(const std::string &modName_, bool force_= false);
  static void removeMod(const std::string &modName_);
  static void removeAllMods(bool force_ = false);
  static void checkAllMods();
  static std::string getModStatus(const std::string &modName_);
  static void apply_mods_list(std::vector<std::string>& modsList_);

public:
  GuiModManager() = default;

  void startApplyModThread(const std::string& modName_);
  void startRemoveModThread(const std::string& modName_);
  void startCheckAllModsThread();
  void startRemoveAllModsThread();
  void startApplyModPresetThread(const std::string &modPresetName_);

protected:
  bool applyModFunction(const std::string& modName_);
  bool applyModPresetFunction(const std::string& presetName_);
  bool removeModFunction(const std::string& modName_);
  bool checkAllModsFunction();
  bool removeAllModsFunction();


private:
  std::string _modName_;

  PopupLoadingBox _loadingBox_;
  std::future<bool> _asyncResponse_{};

};


#endif //SIMPLEMODMANAGER_GUIMODMANAGER_H
