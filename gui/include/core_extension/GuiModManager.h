//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_GUIMODMANAGER_H
#define SIMPLEMODMANAGER_GUIMODMANAGER_H

#include <string>
#include <borealis.hpp>
#include <PopupLoading.h>
#include <future>

class GuiModManager {

public:
  static PopupLoading* _staticPopupLoadingViewPtr_;
  static std::function<void(void)> _staticOnCallBackFunction_;
  static std::vector<std::string> _ignored_file_list_;

  static void applyMod(std::string &modName_, bool force_= false);
  static void removeMod(std::string &modName_);
  static void removeAllMods(bool force_ = false);
  static void checkAllMods();
  static std::string getModStatus(const std::string &modName_);
  static void apply_mods_list(std::vector<std::string>& modsList_);

  static void setOnCallBackFunction(std::function<void(void)> staticOnCallBackFunction_);

public:
  GuiModManager();

  void setModName(std::string modName_);

  void start_apply_mod();
  void start_remove_mod();
  void start_check_all_mods();
  void start_remove_all_mods();
  void start_apply_mod_preset(std::string modPresetName);


private:
  std::string _modName_;
  std::function<bool(std::string mod_name_, brls::Dialog* hostDialogBox_)> _asyncApplyModFunction_;
  std::function<bool(std::string mod_name_, brls::Dialog* hostDialogBox_)> _asyncRemoveModFunction_;
  std::function<bool(std::string mods_preset_name_, brls::Dialog* hostDialogBox_)> _asyncApplyModPresetFunction_;
  std::function<bool(brls::Dialog* hostDialogBox_)> _asyncCheckAllModsFunction_;
  std::function<bool(brls::Dialog* hostDialogBox_)> _asyncRemoveAllModsFunction_;
  std::future<bool> _asyncResponse_;
  PopupLoading* _popupLoadingView_;
  brls::Dialog* _hostDialogBox_;

};


#endif //SIMPLEMODMANAGER_GUIMODMANAGER_H
