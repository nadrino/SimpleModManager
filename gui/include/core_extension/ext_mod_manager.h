//
// Created by Adrien BLANCHET on 28/06/2020.
//

#ifndef SIMPLEMODMANAGER_EXT_MOD_MANAGER_H
#define SIMPLEMODMANAGER_EXT_MOD_MANAGER_H

#include <string>
#include <borealis.hpp>
#include <popup_loading.h>

class ext_mod_manager {

public:
  static popup_loading* _staticPopupLoadingView_;

  static void apply_mod(std::string &modName_, bool force_= false);
  static void remove_mod(std::string &modName_);
  static void check_mods_list(std::vector<std::string>& modsList_);
  static std::string check_mod_status(std::string modName_);

public:
  ext_mod_manager();

  void setModName(std::string modName_);

  void start_apply_mod();
  void start_remove_mod();
  void start_check_mods_list(std::vector<std::string>& modsList_);


private:
  std::string _modName_;
  std::function<bool(std::string mod_name_, brls::Dialog* hostDialogBox_)> _asyncApplyModFunction_;
  std::function<bool(std::string mod_name_, brls::Dialog* hostDialogBox_)> _asyncRemoveModFunction_;
  std::function<bool(std::vector<std::string> mod_name_, brls::Dialog* hostDialogBox_)> _asyncCheckModsListFunction_;
  std::future<bool> _asyncResponse_;
  popup_loading* _popupLoadingView_;
  brls::Dialog* _hostDialogBox_;

};


#endif //SIMPLEMODMANAGER_EXT_MOD_MANAGER_H
