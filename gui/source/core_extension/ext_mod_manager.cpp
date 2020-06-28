//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_mod_manager.h"
#include <GlobalObjects.h>

#include <ext_toolbox.h>
#include <tab_mod_browser.h>
#include <ext_GlobalObjects.h>


// static
popup_loading* ext_mod_manager::_staticPopupLoadingView_;

void ext_mod_manager::apply_mod(std::string &modName_, bool force_) {

  std::string absolute_mod_folder_path = GlobalObjects::get_mod_browser().get_mod_manager().get_current_mods_folder_path() + "/" + modName_;

  if(ext_mod_manager::_staticPopupLoadingView_) ext_mod_manager::_staticPopupLoadingView_->setSubTitle("Getting files list...");
  std::vector<std::string> relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

  // deleting ignored entries
  for(int i_mod = int(relative_file_path_list.size())-1 ; i_mod >= 0 ; i_mod--){
    if(toolbox::do_string_in_vector(relative_file_path_list[i_mod], GlobalObjects::get_mod_browser().get_mod_manager().get_ignored_file_list())){
      relative_file_path_list.erase(relative_file_path_list.begin() + i_mod);
    }
  }

  std::string replace_option;
  if(force_) replace_option = "Yes to all";
  bool is_conflict;
  std::stringstream ss_files_list;

  for(int i_file = 0 ; i_file < int(relative_file_path_list.size()) ; i_file++){

    if(relative_file_path_list[i_file][0] == '.'){
      // ignoring cached files
      continue;
    }

    if(ext_mod_manager::_staticPopupLoadingView_){
      ext_mod_manager::_staticPopupLoadingView_->setSubTitle(toolbox::get_filename_from_file_path(relative_file_path_list[i_file]));
      ext_mod_manager::_staticPopupLoadingView_->setProgressFraction((i_file+1.)/double(relative_file_path_list.size()));
    }

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    std::string install_path = GlobalObjects::get_mod_browser().get_mod_manager().get_install_mods_base_folder() + "/" + relative_file_path_list[i_file];
    if(toolbox::do_path_is_file(install_path)) {
      is_conflict = true;
//      if (replace_option == "Yes to all") {
//         remove log entry ? if log enabled
//      }
//      else if (replace_option == "No to all") {
//        continue; // do nothing
//      }
//      else {
//        replace_option = ask_to_replace(relative_file_path_list[i_file]);
//        std::cout << ss_files_list.str();
//      }
    }
    else {
      is_conflict = false;
    }
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      toolbox::copy_file( absolute_file_path, install_path );
    }

  }
  GlobalObjects::get_mod_browser().get_mod_manager().reset_mod_cache_status(modName_);


}
std::string ext_mod_manager::check_mod_status(std::string modName_) {

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE

  auto* mod_manager = &GlobalObjects::get_mod_browser().get_mod_manager();

  if(not mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_].empty())
    return mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_];

  if(mod_manager->isUseCacheOnlyForStatusCheck()) return "Not Checked";

  std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

  int same_files_count = 0;

  if(ext_mod_manager::_staticPopupLoadingView_) ext_mod_manager::_staticPopupLoadingView_->setSubTitle("Listing mod files...");
  std::vector<std::string> relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

  int total_files_count = relative_file_path_list.size();
  toolbox::reset_last_displayed_value();
  for(int i_file = 0 ; i_file < total_files_count ; i_file++){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

    if(ext_mod_manager::_staticPopupLoadingView_){
      ext_mod_manager::_staticPopupLoadingView_->setSubTitle(toolbox::get_filename_from_file_path(relative_file_path_list[i_file]));
      ext_mod_manager::_staticPopupLoadingView_->setProgressFraction((i_file+1.)/double(total_files_count));
    }

    if(toolbox::do_files_are_the_same(
      mod_manager->get_install_mods_base_folder() + "/" + relative_file_path_list[i_file],
      absolute_file_path
    )) same_files_count++;
  }

  mod_manager->getModsStatusCacheFraction()
  [mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_]
    = double(same_files_count) / double(total_files_count);

  std::string result;
  if(same_files_count == total_files_count) result = "ACTIVE";
  else if(same_files_count == 0) result = "INACTIVE";
  else result = "PARTIAL (" + std::to_string(same_files_count) + "/" + std::to_string(total_files_count) + ")";

  mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_] = result;

  mod_manager->save_mods_status_cache_file();
  std::cout << "_currentTabModBrowserPtr_ = " << ext_GlobalObjects::getCurrentTabModBrowserPtr() << std::endl;
  if(ext_GlobalObjects::getCurrentTabModBrowserPtr()) ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[modName_]->setValue(result);
  return result;

}
void ext_mod_manager::check_mods_list(std::vector<std::string> &modsList_) {

  auto* mod_browser = &GlobalObjects::get_mod_browser();

  mod_browser->get_selector().reset_tags_list();
  auto mods_list = mod_browser->get_selector().get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){

    if(ext_mod_manager::_staticPopupLoadingView_){
      ext_mod_manager::_staticPopupLoadingView_->setSubTitle(toolbox::get_filename_from_file_path(mods_list[i_mod]));
      ext_mod_manager::_staticPopupLoadingView_->setProgressFraction((i_mod+1.)/double(mods_list.size()));
    }

    hidScanInput();
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    if(kDown & KEY_B) break;

    mod_browser->get_selector().set_tag(i_mod, "Checking...");
    std::string result = mod_browser->get_mod_manager().get_mod_status(mods_list[i_mod]);
    mod_browser->get_selector().set_tag(i_mod, result);
    if(ext_GlobalObjects::getCurrentTabModBrowserPtr()) ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValue(result);
  }
}
void ext_mod_manager::remove_mod(std::string &modName_){

  auto* mod_manager = &GlobalObjects::get_mod_browser().get_mod_manager();

  if(ext_mod_manager::_staticPopupLoadingView_) ext_mod_manager::_staticPopupLoadingView_->setSubTitle("Disabling: " + modName_);
  std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

  std::vector<std::string> relative_file_path_list;

  relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

  int i_file=0;
  toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    if(ext_mod_manager::_staticPopupLoadingView_){
      ext_mod_manager::_staticPopupLoadingView_->setSubTitle(toolbox::get_filename_from_file_path(relative_file_path));
      ext_mod_manager::_staticPopupLoadingView_->setProgressFraction((i_file+1.)/double(relative_file_path_list.size()));
    }

    i_file++;
    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    absolute_file_path = toolbox::remove_extra_doubled_characters(absolute_file_path, "/");
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    std::string installed_file_path = mod_manager->get_install_mods_base_folder() + "/" + relative_file_path;
    installed_file_path = toolbox::remove_extra_doubled_characters(installed_file_path, "/");
    // Check if the installed mod belongs to the selected mod
    if( toolbox::do_files_are_the_same( absolute_file_path, installed_file_path ) ){

      // Remove the mod file
      toolbox::delete_file(installed_file_path);

      // Delete the folder if no other files is present
      std::string empty_folder_path_candidate = toolbox::get_folder_path_from_file_path(installed_file_path);
      while( toolbox::do_folder_is_empty( empty_folder_path_candidate ) ) {

        toolbox::delete_directory(empty_folder_path_candidate);

        std::vector<std::string> sub_folder_list = toolbox::split_string(empty_folder_path_candidate, "/");
        if(sub_folder_list.empty()) break; // virtually impossible -> would mean everything has been deleted on the sd
        // decrement folder depth
        empty_folder_path_candidate =
          "/" + toolbox::join_vector_string(
            sub_folder_list,
            "/",
            0,
            int(sub_folder_list.size()) - 1
          );
      }
    }
  }
  mod_manager->reset_mod_cache_status(modName_);

}


// not static
ext_mod_manager::ext_mod_manager() {
  _modName_ = "";
  _popupLoadingView_ = nullptr;
  _hostDialogBox_ = nullptr;

  ext_mod_manager::_staticPopupLoadingView_ = nullptr;

  _asyncApplyModFunction_  = [](std::string mod_name_, brls::Dialog* hostDialogBox_){
    ext_mod_manager::_staticPopupLoadingView_->setTitle("Applying: " + mod_name_);
    ext_mod_manager::_staticPopupLoadingView_->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    ext_mod_manager::_staticPopupLoadingView_->setProgressFraction(0);
    ext_mod_manager::apply_mod(mod_name_, true);
    ext_mod_manager::_staticPopupLoadingView_->setTitle("Checking: " + mod_name_);
    ext_mod_manager::_staticPopupLoadingView_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingView_->setProgressFraction(0);
    ext_mod_manager::check_mod_status(mod_name_);
    brls::Application::popView();
    return true;
  };
  _asyncRemoveModFunction_ = [](std::string mod_name_, brls::Dialog* hostDialogBox_){
    ext_mod_manager::_staticPopupLoadingView_->setTitle("Disabling: " + mod_name_);
    ext_mod_manager::_staticPopupLoadingView_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    ext_mod_manager::_staticPopupLoadingView_->setProgressFraction(0);
    ext_mod_manager::remove_mod(mod_name_);
    ext_mod_manager::_staticPopupLoadingView_->setTitle("Checking: " + mod_name_);
    ext_mod_manager::_staticPopupLoadingView_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingView_->setProgressFraction(0);
    ext_mod_manager::check_mod_status(mod_name_);
    brls::Application::popView();
    return true;
  };
  _asyncCheckModsListFunction_ = [](std::vector<std::string> mod_list_, brls::Dialog* hostDialogBox_){
    ext_mod_manager::_staticPopupLoadingView_->setTitle("Checking " + std::to_string(mod_list_.size()) + " mods.");
    ext_mod_manager::_staticPopupLoadingView_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingView_->setProgressFraction(0);
    ext_mod_manager::check_mods_list(mod_list_);
    brls::Application::popView();
    brls::Application::unblockInputs();
    return true;
  };

}

void ext_mod_manager::setModName(std::string modName_) {
  _modName_ = std::move(modName_);
}

void ext_mod_manager::start_apply_mod() {

  if(_modName_.empty()) return;
  if(_asyncApplyModFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    _staticPopupLoadingView_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncApplyModFunction_, _modName_, _hostDialogBox_);
    _popupLoadingView_->setAsyncResponse(&_asyncResponse_);
  }

}
void ext_mod_manager::start_remove_mod(){

  if(_modName_.empty()) return;
  if(_asyncRemoveModFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    _staticPopupLoadingView_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncRemoveModFunction_, _modName_, _hostDialogBox_);
    _popupLoadingView_->setAsyncResponse(&_asyncResponse_);
  }

}
void ext_mod_manager::start_check_mods_list(std::vector<std::string>& modsList_){
  if(_asyncCheckModsListFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    ext_mod_manager::_staticPopupLoadingView_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncCheckModsListFunction_, modsList_, _hostDialogBox_);
    _popupLoadingView_->setAsyncResponse(&_asyncResponse_);
  }
}





