//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "ext_mod_manager.h"
#include <GlobalObjects.h>

#include <ext_toolbox.h>
#include <tab_mod_browser.h>
#include <ext_GlobalObjects.h>
#include <toolbox.h>

#include <utility>


// static
popup_loading* ext_mod_manager::_staticPopupLoadingViewPtr_;
std::function<void(void)> ext_mod_manager::_staticOnCallBackFunction_;
std::vector<std::string> ext_mod_manager::_ignored_file_list_;

void ext_mod_manager::apply_mod(std::string &modName_, bool force_) {

  std::string absolute_mod_folder_path = GlobalObjects::get_mod_browser().get_mod_manager().get_current_mods_folder_path() + "/" + modName_;

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

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    toolbox::fill_str_buffer_map("ext_mod_manager::apply_mod:current_file", toolbox::get_filename_from_file_path(relative_file_path_list[i_file]) + " (" + file_size + ")");
    toolbox::fill_progress_map("ext_mod_manager::apply_mod", (i_file+1.)/double(relative_file_path_list.size()));

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

  if(mod_manager->isUseCacheOnlyForStatusCheck()){
    return "Not Checked";
  }
  else if(not mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_].empty()) {
    return mod_manager->get_mods_status_cache()[
      mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_];
  }
  else{

    std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

    int same_files_count = 0;

    toolbox::fill_str_buffer_map("ext_mod_manager::check_mod_status:current_file", "Listing mod files...");
    std::vector<std::string> relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

    int total_files_count = relative_file_path_list.size();
    for(int i_file = 0 ; i_file < total_files_count ; i_file++){

      std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

      toolbox::fill_str_buffer_map("ext_mod_manager::check_mod_status:current_file", toolbox::get_filename_from_file_path(relative_file_path_list[i_file]));
      toolbox::fill_progress_map("ext_mod_manager::check_mod_status", (i_file+1.)/double(total_files_count));

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
    if(ext_GlobalObjects::getCurrentTabModBrowserPtr()) ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[modName_]->setValue(result);

    return result;
  }

}
void ext_mod_manager::remove_mod(std::string &modName_){

  auto* mod_manager = &GlobalObjects::get_mod_browser().get_mod_manager();

  std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

  std::vector<std::string> relative_file_path_list;

  relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

  int i_file=0;
  toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    toolbox::fill_str_buffer_map("ext_mod_manager::remove_mod:current_file",
      toolbox::get_filename_from_file_path(relative_file_path)
      + " (" + toolbox::get_file_size_string(absolute_file_path) + ")");
    toolbox::fill_progress_map("ext_mod_manager::remove_mod", (i_file+1.)/double(relative_file_path_list.size()));

    i_file++;
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
void ext_mod_manager::remove_all_mods(bool force_) {

  auto mods_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();

  std::string answer;

  if(not force_){
//    answer = toolbox::ask_question(
//      "Do you want to disable all mods ?",
//      std::vector<std::string>({"Yes", "No"})
//    );
  }
  else{
    answer = "Yes";
  }

  if(answer == "Yes") {
    for( int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++ ){
      toolbox::fill_str_buffer_map("ext_mod_manager::remove_all_mods:current_mod", mods_list[i_mod]);
      toolbox::fill_progress_map("ext_mod_manager::remove_all_mods", (i_mod+1.)/double(mods_list.size()));
      ext_mod_manager::remove_mod(mods_list[i_mod]);
    }
  }

}
void ext_mod_manager::apply_mods_list(std::vector<std::string>& modsList_){

  auto* mod_browser = &GlobalObjects::get_mod_browser();

  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(modsList_.size());
  for(int i_mod = int(modsList_.size())-1 ; i_mod >= 0 ; i_mod--){
    std::string mod_path = mod_browser->get_current_directory() + "/" + modsList_[i_mod];
    auto mod_files_list = toolbox::get_list_files_in_subfolders(mod_path);
    for(auto& mod_file : mod_files_list){
      if(toolbox::do_string_in_vector(mod_file, applied_files_listing)){
        mods_ignored_files_list[i_mod].emplace_back(mod_file);
      }
      else {
        applied_files_listing.emplace_back(mod_file);
      }
    }
  }

  // applying mods with ignored files
  for(int i_mod = 0 ; i_mod < int(modsList_.size()) ; i_mod++){
    toolbox::fill_str_buffer_map("ext_mod_manager::apply_mods_list:current_mod",
      modsList_[i_mod] + " (" + std::to_string(i_mod+1) + "/" + std::to_string(modsList_.size()) + ")");
    toolbox::fill_progress_map("ext_mod_manager::apply_mods_list", (i_mod+1.)/double(modsList_.size()));
    mod_browser->get_mod_manager().set_ignored_file_list(mods_ignored_files_list[i_mod]);
    ext_mod_manager::apply_mod(modsList_[i_mod], true); // normally should work without force (tested) but just in case...
    mod_browser->get_mod_manager().get_ignored_file_list().clear();
  }

}
void ext_mod_manager::check_all_mods() {

  auto* mod_browser = &GlobalObjects::get_mod_browser();
  auto modsList = mod_browser->get_selector().get_selection_list();

  mod_browser->get_selector().reset_tags_list();
  auto mods_list = mod_browser->get_selector().get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){

    toolbox::fill_str_buffer_map("ext_mod_manager::check_all_mods:current_mod", toolbox::get_filename_from_file_path(mods_list[i_mod]));
    toolbox::fill_progress_map("ext_mod_manager::check_all_mods", (i_mod+1.)/double(mods_list.size()));

    // IU update
    mod_browser->get_selector().set_tag(i_mod, "Checking...");
    std::string result = ext_mod_manager::check_mod_status(mods_list[i_mod]); // actual mod check is here
    mod_browser->get_selector().set_tag(i_mod, result);
    // TODO: Change this to a boolean that triggers in the draw loop ?
    if(ext_GlobalObjects::getCurrentTabModBrowserPtr()) ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValue(result);

  }

}

void ext_mod_manager::setOnCallBackFunction(std::function<void(void)> staticOnCallBackFunction_){
  ext_mod_manager::_staticOnCallBackFunction_ = std::move(staticOnCallBackFunction_);
}

// not static
ext_mod_manager::ext_mod_manager() {
  _modName_ = "";
  _popupLoadingView_ = nullptr;
  _hostDialogBox_ = nullptr;

  ext_mod_manager::_staticPopupLoadingViewPtr_ = nullptr;

  _asyncApplyModFunction_  = [](std::string mod_name_, brls::Dialog* hostDialogBox_){

    brls::Logger::info("Applying: %s", mod_name_.c_str());
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Applying mod...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::apply_mod:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::apply_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("copy_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::apply_mod(mod_name_, true);

    brls::Logger::info("Checking: %s", mod_name_.c_str());
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Checking the applied mod...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_mod_status:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_mod_status"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("do_files_are_the_same"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::check_mod_status(mod_name_);

    if(ext_mod_manager::_staticOnCallBackFunction_ ? true: false){
      brls::Application::popView(brls::ViewAnimation::FADE, ext_mod_manager::_staticOnCallBackFunction_);
      ext_mod_manager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };
  _asyncRemoveModFunction_ = [](std::string mod_name_, brls::Dialog* hostDialogBox_){

    brls::Logger::info("Removing: %s", mod_name_.c_str());
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Removing mod...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    ext_mod_manager::remove_mod(mod_name_);

    brls::Logger::info("Checking: %s", mod_name_.c_str());
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Checking mod...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_mod_status:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_mod_status"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("do_files_are_the_same"));
    ext_mod_manager::check_mod_status(mod_name_);

    // check if the call back function is callable
    if(ext_mod_manager::_staticOnCallBackFunction_ ? true: false){
      brls::Application::popView(brls::ViewAnimation::FADE, ext_mod_manager::_staticOnCallBackFunction_);
      ext_mod_manager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };

  _asyncCheckAllModsFunction_ = [](brls::Dialog* hostDialogBox_){

    brls::Logger::info("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_all_mods"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_mod_status:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_mod_status"));
    ext_mod_manager::check_all_mods();

    if(ext_mod_manager::_staticOnCallBackFunction_ ? true: false){
      brls::Application::popView(brls::ViewAnimation::FADE, ext_mod_manager::_staticOnCallBackFunction_);
      ext_mod_manager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };
  _asyncRemoveAllModsFunction_ = [](brls::Dialog* hostDialogBox_){

    brls::Logger::info("Removing all installed mods...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Removing all installed mods...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_all_mods:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_all_mods"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    ext_mod_manager::remove_all_mods(true);

    brls::Logger::info("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_mod_status:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_all_mods"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_mod_status"));
    ext_mod_manager::check_all_mods();

    if(ext_mod_manager::_staticOnCallBackFunction_ ? true: false){
      brls::Application::popView(brls::ViewAnimation::FADE, ext_mod_manager::_staticOnCallBackFunction_);
      ext_mod_manager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };
  _asyncApplyModPresetFunction_ = [](std::string mod_preset_name_, brls::Dialog* hostDialogBox_){

    brls::Logger::info("Removing all installed mods...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Removing all installed mods...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_all_mods:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_all_mods"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    ext_mod_manager::remove_all_mods(true);

    brls::Logger::info("Applying Mod Preset");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Applying mod preset...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::apply_mods_list:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::apply_mod:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::apply_mods_list"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::apply_mod"));
    auto modsList = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(mod_preset_name_);
    ext_mod_manager::apply_mods_list(modsList);

    brls::Logger::info("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    ext_mod_manager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_mod_status:current_file"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_all_mods"));
    ext_mod_manager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    ext_mod_manager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::check_mod_status"));
    ext_mod_manager::check_all_mods();

    if(ext_mod_manager::_staticOnCallBackFunction_ ? true: false){
      brls::Application::popView(brls::ViewAnimation::FADE, ext_mod_manager::_staticOnCallBackFunction_);
      ext_mod_manager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    ext_mod_manager::_staticPopupLoadingViewPtr_->reset();
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
    _staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncApplyModFunction_, _modName_, _hostDialogBox_);
  }

}
void ext_mod_manager::start_remove_mod(){

  if(_modName_.empty()) return;
  if(_asyncRemoveModFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    _staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncRemoveModFunction_, _modName_, _hostDialogBox_);
  }

}
void ext_mod_manager::start_check_all_mods(){
  if(_asyncCheckAllModsFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    ext_mod_manager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncCheckAllModsFunction_, _hostDialogBox_);
  }
}
void ext_mod_manager::start_remove_all_mods(){
  if(_asyncCheckAllModsFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    ext_mod_manager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncRemoveAllModsFunction_, _hostDialogBox_);
  }
}
void ext_mod_manager::start_apply_mod_preset(std::string modPresetName){
  if(_asyncCheckAllModsFunction_ ? true : false){
    _popupLoadingView_ = new popup_loading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    ext_mod_manager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncApplyModPresetFunction_, modPresetName, _hostDialogBox_);
  }
}




