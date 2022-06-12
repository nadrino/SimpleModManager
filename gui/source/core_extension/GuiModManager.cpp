//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "GuiModManager.h"
#include <GlobalObjects.h>
#include <TabModBrowser.h>
#include <ext_GlobalObjects.h>
#include <toolbox.h>

#include "GenericToolbox.Switch.h"
#include "Logger.h"

#include <utility>
#include "string"
#include "vector"
#include "future"


LoggerInit([]{
  Logger::setUserHeaderStr("[GuiModManager]");
});

// static
PopupLoading* GuiModManager::_staticPopupLoadingViewPtr_;
std::function<void(void)> GuiModManager::_staticOnCallBackFunction_;
std::vector<std::string> GuiModManager::_ignored_file_list_;

void GuiModManager::applyMod(std::string &modName_, bool force_) {

  std::string absolute_mod_folder_path = GlobalObjects::get_mod_browser().get_mod_manager().get_current_mods_folder_path() + "/" + modName_;

  std::vector<std::string> relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

  // deleting ignored entries
  for(int i_mod = int(relative_file_path_list.size())-1 ; i_mod >= 0 ; i_mod--){
    if(GenericToolbox::doesElementIsInVector(relative_file_path_list[i_mod], GlobalObjects::get_mod_browser().get_mod_manager().get_ignored_file_list())){
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
    GenericToolbox::removeRepeatedCharacters(absolute_file_path, "/");
    std::string file_size = toolbox::get_file_size_string(absolute_file_path);

    toolbox::fill_str_buffer_map("ext_mod_manager::applyMod:current_file", toolbox::get_filename_from_file_path(relative_file_path_list[i_file]) + " (" + file_size + ")");
    toolbox::fill_progress_map("ext_mod_manager::applyMod", (i_file+1.)/double(relative_file_path_list.size()));

    std::string install_path = GlobalObjects::get_mod_browser().get_mod_manager().get_install_mods_base_folder() + "/" + relative_file_path_list[i_file];
    GenericToolbox::removeRepeatedCharInsideInputStr(install_path, "/");
    GenericToolbox::Switch::IO::doesPathIsFile(install_path) ? is_conflict = true: is_conflict = false;
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      GenericToolbox::Switch::IO::copyFile(absolute_file_path, install_path);
    }

  }
  GlobalObjects::get_mod_browser().get_mod_manager().reset_mod_cache_status(modName_);

}
std::string GuiModManager::getModStatus(const std::string &modName_) {
  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE
  std::string result;

  auto* mod_manager = &GlobalObjects::get_mod_browser().get_mod_manager();

  if(mod_manager->isUseCacheOnlyForStatusCheck()){
    result = "Not Checked";
  }
  else if(not mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_].empty()) {
    result = mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_];
  }
  else{

    std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

    int same_files_count = 0;

    toolbox::fill_str_buffer_map("ext_mod_manager::getModStatus:current_file", "Listing mod files...");
    std::vector<std::string> relative_file_path_list = toolbox::get_list_files_in_subfolders(absolute_mod_folder_path);

    int total_files_count = relative_file_path_list.size();
    for(int i_file = 0 ; i_file < total_files_count ; i_file++){

      std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

      toolbox::fill_str_buffer_map("ext_mod_manager::getModStatus:current_file", toolbox::get_filename_from_file_path(relative_file_path_list[i_file]));
      toolbox::fill_progress_map("ext_mod_manager::getModStatus", (i_file+1.)/double(total_files_count));

      if(GenericToolbox::Switch::IO::doFilesAreIdentical(
        mod_manager->get_install_mods_base_folder() + "/" + relative_file_path_list[i_file],
        absolute_file_path
      )){ same_files_count++; }

    }

    mod_manager->getModsStatusCacheFraction()[
      mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_
      ] = double(same_files_count) / double(total_files_count);

    if(same_files_count == total_files_count) result = "ACTIVE";
    else if(same_files_count == 0) result = "INACTIVE";
    else result = "PARTIAL (" + std::to_string(same_files_count) + "/" + std::to_string(total_files_count) + ")";

    mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_] = result;
    mod_manager->save_mods_status_cache_file();

  }

  // the status will be updated on the next rendered frame
  ext_GlobalObjects::getCurrentTabModBrowserPtr()->setTriggerUpdateModsDisplayedStatus(true);

  return result;
}
void GuiModManager::remove_mod(std::string &modName_){

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
    if( GenericToolbox::Switch::IO::doFilesAreIdentical( absolute_file_path, installed_file_path ) ){

      // Remove the mod file
      GenericToolbox::Switch::IO::deleteFile(installed_file_path);

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
void GuiModManager::remove_all_mods(bool force_) {

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
      toolbox::fill_str_buffer_map("ext_mod_manager::remove_all_mods:current_mod",
        mods_list[i_mod] + " (" + std::to_string(i_mod+1) + "/" + std::to_string(mods_list.size()) + ")");
      toolbox::fill_progress_map("ext_mod_manager::remove_all_mods", (i_mod+1.)/double(mods_list.size()));
      GuiModManager::remove_mod(mods_list[i_mod]);
    }
  }

}
void GuiModManager::apply_mods_list(std::vector<std::string>& modsList_){

  auto* mod_browser = &GlobalObjects::get_mod_browser();

  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(modsList_.size());
  for(int i_mod = int(modsList_.size())-1 ; i_mod >= 0 ; i_mod--){
    std::string mod_path = mod_browser->get_current_directory() + "/" + modsList_[i_mod];
    auto mod_files_list = toolbox::get_list_files_in_subfolders(mod_path);
    for(auto& mod_file : mod_files_list){
      if(GenericToolbox::doesElementIsInVector(mod_file, applied_files_listing)){
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
    GuiModManager::applyMod(modsList_[i_mod], true); // normally should work without force (tested) but just in case...
    mod_browser->get_mod_manager().get_ignored_file_list().clear();
  }

}
void GuiModManager::check_all_mods() {

  auto* mod_browser = &GlobalObjects::get_mod_browser();
  auto modsList = mod_browser->get_selector().get_selection_list();

  mod_browser->get_selector().reset_tags_list();
  auto mods_list = mod_browser->get_selector().get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){

    toolbox::fill_str_buffer_map("ext_mod_manager::check_all_mods:current_mod",
      toolbox::get_filename_from_file_path(mods_list[i_mod]) + " (" + std::to_string(i_mod+1) + "/" + std::to_string(mods_list.size()) + ")");
    toolbox::fill_progress_map("ext_mod_manager::check_all_mods", (i_mod+1.)/double(mods_list.size()));

    // IU update
    std::string result = GuiModManager::getModStatus(mods_list[i_mod]); // actual mod check is here
    // TODO: Change this to a boolean that triggers in the draw loop ?
//    if(ext_GlobalObjects::getCurrentTabModBrowserPtr()){
//      ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValue(result);
//      NVGcolor color;
//      if(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status_fraction(mods_list[i_mod]) == 0){
//        color.a = 255./255.; color.r = 80./255.; color.g = 80./255.; color.b = 80./255.;
//        ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          color
//          );
//      }
//      else if(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status_fraction(mods_list[i_mod]) == 1){
//        color.a = 255./255.; color.r = 43./255.; color.g = 81./255.; color.b = 226./255.;
//        ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          color
//        );
//      }
//      else{
//        ext_GlobalObjects::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          {43, 81, 226, 255}
//        );
//      }
//    }

  }

}

void GuiModManager::setOnCallBackFunction(std::function<void(void)> staticOnCallBackFunction_){
  GuiModManager::_staticOnCallBackFunction_ = std::move(staticOnCallBackFunction_);
}

// not static
GuiModManager::GuiModManager() {
  _modName_ = "";
  _popupLoadingView_ = nullptr;
  _hostDialogBox_ = nullptr;

  GuiModManager::_staticPopupLoadingViewPtr_ = nullptr;

  _asyncApplyModFunction_  = [](std::string mod_name_, brls::Dialog* hostDialogBox_){

    LogInfo("Applying: %s", mod_name_.c_str());

    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Applying mod...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::applyMod:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::applyMod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::applyMod(mod_name_, true);

    LogInfo("Checking: %s", mod_name_.c_str());
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Checking the applied mod...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::getModStatus:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::getModStatus"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::getModStatus(mod_name_);

    if(GuiModManager::_staticOnCallBackFunction_ ? true : false){
      brls::Application::popView(brls::ViewAnimation::FADE, GuiModManager::_staticOnCallBackFunction_);
      GuiModManager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };
  _asyncRemoveModFunction_ = [](std::string mod_name_, brls::Dialog* hostDialogBox_){

    LogInfo("Removing: %s", mod_name_.c_str());
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Removing mod...");
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    GuiModManager::remove_mod(mod_name_);

    LogInfo("Checking: %s", mod_name_.c_str());
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Checking mod...");
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&mod_name_);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::getModStatus:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::getModStatus"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    GuiModManager::getModStatus(mod_name_);

    // check if the call back function is callable
    if(GuiModManager::_staticOnCallBackFunction_ ? true : false){
      brls::Application::popView(brls::ViewAnimation::FADE, GuiModManager::_staticOnCallBackFunction_);
      GuiModManager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };

  _asyncCheckAllModsFunction_ = [](brls::Dialog* hostDialogBox_){

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    LogInfo("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::getModStatus"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::getModStatus:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    GuiModManager::check_all_mods();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // 60fps = 17ms per frame
    auto processingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    LogDebug("processingDuration = %i ms", int(processingDuration));
    if(processingDuration < 17*5){ // wait at least 5 frames
      LogDebug("extra wait... %i ms", int(17*5-processingDuration));
      std::chrono::milliseconds interval(17*5 - processingDuration);
      std::this_thread::sleep_for(interval);
      LogDebug("done");
    }

    if(GuiModManager::_staticOnCallBackFunction_){
      brls::Application::popView(brls::ViewAnimation::SLIDE_LEFT, GuiModManager::_staticOnCallBackFunction_);
      GuiModManager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    GuiModManager::_staticPopupLoadingViewPtr_->reset();

    return true;
  };
  _asyncRemoveAllModsFunction_ = [](brls::Dialog* hostDialogBox_){

    LogInfo("Removing all installed mods...");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Removing all installed mods...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_all_mods:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_all_mods"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    GuiModManager::remove_all_mods(true);

    LogInfo("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::getModStatus:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::getModStatus"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    GuiModManager::check_all_mods();

    if(GuiModManager::_staticOnCallBackFunction_){
      brls::Application::popView(brls::ViewAnimation::FADE, GuiModManager::_staticOnCallBackFunction_);
      GuiModManager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };
  _asyncApplyModPresetFunction_ = [](std::string mod_preset_name_, brls::Dialog* hostDialogBox_){

    LogInfo("Removing all installed mods...");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Removing all installed mods...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_all_mods:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::remove_mod:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_all_mods"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::remove_mod"));
    GuiModManager::remove_all_mods(true);

    LogInfo("Applying Mod Preset");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Applying mod preset...");
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::apply_mods_list:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::applyMod:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::applyMod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);
    auto modsList = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(mod_preset_name_);
    GuiModManager::apply_mods_list(modsList);

    LogInfo("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    GuiModManager::_staticPopupLoadingViewPtr_->setHeader("Checking all mods status...");
    GuiModManager::_staticPopupLoadingViewPtr_->setTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::check_all_mods:current_mod"));
    GuiModManager::_staticPopupLoadingViewPtr_->setSubTitlePtr(&toolbox::get_str_buffer("ext_mod_manager::getModStatus:current_file"));
    GuiModManager::_staticPopupLoadingViewPtr_->setProgressFractionPtr(&toolbox::get_progress("ext_mod_manager::getModStatus"));
    GuiModManager::_staticPopupLoadingViewPtr_->setEnableSubLoadingBar(true);
    GuiModManager::_staticPopupLoadingViewPtr_->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    GuiModManager::check_all_mods();

    if(GuiModManager::_staticOnCallBackFunction_){
      brls::Application::popView(brls::ViewAnimation::FADE, GuiModManager::_staticOnCallBackFunction_);
      GuiModManager::setOnCallBackFunction([](){}); // reset
    }
    else{
      brls::Application::popView(brls::ViewAnimation::FADE);
    }

    brls::Application::unblockInputs();
    GuiModManager::_staticPopupLoadingViewPtr_->reset();
    return true;
  };

}

void GuiModManager::setModName(std::string modName_) {
  _modName_ = std::move(modName_);
}

void GuiModManager::start_apply_mod() {

  if(_modName_.empty()) return;
  if(_asyncApplyModFunction_){
    _popupLoadingView_ = new PopupLoading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    _staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncApplyModFunction_, _modName_, _hostDialogBox_);
  }

}
void GuiModManager::start_remove_mod(){

  if(_modName_.empty()) return;
  if(_asyncRemoveModFunction_){
    _popupLoadingView_ = new PopupLoading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    _staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncRemoveModFunction_, _modName_, _hostDialogBox_);
  }

}
void GuiModManager::start_check_all_mods(){
  if(_asyncCheckAllModsFunction_){
    LogInfo("Async check all mods function is starting...");
    _popupLoadingView_ = new PopupLoading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_, brls::ViewAnimation::SLIDE_RIGHT);
    GuiModManager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncCheckAllModsFunction_, _hostDialogBox_);
  }
}
void GuiModManager::start_remove_all_mods(){
  if(_asyncCheckAllModsFunction_){
    _popupLoadingView_ = new PopupLoading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    GuiModManager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncRemoveAllModsFunction_, _hostDialogBox_);
  }
}
void GuiModManager::start_apply_mod_preset(std::string modPresetName){
  if(_asyncCheckAllModsFunction_ ? true : false){
    _popupLoadingView_ = new PopupLoading();
    _hostDialogBox_ = new brls::Dialog(_popupLoadingView_);
    brls::Application::pushView(_hostDialogBox_);
    GuiModManager::_staticPopupLoadingViewPtr_ = _popupLoadingView_;
    _asyncResponse_ = std::async(std::launch::async, _asyncApplyModPresetFunction_, modPresetName, _hostDialogBox_);
  }
}




