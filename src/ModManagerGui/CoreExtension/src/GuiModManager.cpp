//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "GuiModManager.h"
#include <GlobalObjects.h>
#include <Toolbox.h>

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
void GuiModManager::applyMod(const std::string &modName_, bool force_) {

  std::string modPath = GlobalObjects::getModBrowser().getModManager().get_current_mods_folder_path() + "/" + modName_;

  std::vector<std::string> modFilesList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  // deleting ignored entries
  for(int i_mod = int(modFilesList.size()) - 1 ; i_mod >= 0 ; i_mod--){
    if(GenericToolbox::doesElementIsInVector(modFilesList[i_mod], GlobalObjects::getModBrowser().getModManager().get_ignored_file_list())){
      modFilesList.erase(modFilesList.begin() + i_mod);
    }
  }

  std::string replace_option;
  if(force_) replace_option = "Yes to all";
  bool is_conflict;
  std::stringstream ss_files_list;

  for(size_t iFile = 0 ; iFile < modFilesList.size() ; iFile++){

    if(modFilesList[iFile][0] == '.'){
      // ignoring cached files
      continue;
    }

    std::string filePath = modPath + "/" + modFilesList[iFile];
    GenericToolbox::removeRepeatedCharacters(filePath, "/");
    std::string fileSizeStr = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(filePath)));

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyMod:current_file"] = GenericToolbox::getFileNameFromFilePath(modFilesList[iFile]) + " (" + fileSizeStr + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyMod"] = double(iFile + 1) / double(modFilesList.size());

    std::string install_path =
        GlobalObjects::getModBrowser().getModManager().get_install_mods_base_folder() + "/" + modFilesList[iFile];
    GenericToolbox::removeRepeatedCharInsideInputStr(install_path, "/");
    GenericToolbox::doesPathIsFile(install_path) ? is_conflict = true: is_conflict = false;
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      GenericToolbox::Switch::IO::copyFile(filePath, install_path);
    }

  }
  GlobalObjects::getModBrowser().getModManager().reset_mod_cache_status(modName_);

}
std::string GuiModManager::getModStatus(const std::string &modName_) {
  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE
  std::string result;

  auto* mod_manager = &GlobalObjects::getModBrowser().getModManager();

  if(mod_manager->isUseCacheOnlyForStatusCheck()){
    result = "Not Checked";
  }
  else if(not mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_].empty()) {
    result = mod_manager->get_mods_status_cache()[mod_manager->getParametersHandlerPtr()->get_current_config_preset_name() + ": " + modName_];
  }
  else{

    std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

    int same_files_count = 0;

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"] = "Listing mod files...";
    std::vector<std::string> relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(
        absolute_mod_folder_path);

    int total_files_count = relative_file_path_list.size();
    for(int i_file = 0 ; i_file < total_files_count ; i_file++){

      std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path_list[i_file];

      GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"] = GenericToolbox::getFileNameFromFilePath(relative_file_path_list[i_file]);
      GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"] = (i_file + 1.) / double(total_files_count);

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

  return result;
}
void GuiModManager::removeMod(const std::string &modName_){

  auto* mod_manager = &GlobalObjects::getModBrowser().getModManager();

  std::string absolute_mod_folder_path = mod_manager->get_current_mods_folder_path() + "/" + modName_;

  std::vector<std::string> relative_file_path_list;

  relative_file_path_list = GenericToolbox::getListOfFilesInSubFolders(absolute_mod_folder_path);

  int i_file=0;
  Toolbox::reset_last_displayed_value();
  for(auto &relative_file_path : relative_file_path_list){

    std::string absolute_file_path = absolute_mod_folder_path + "/" + relative_file_path;
    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeMod:current_file"] =
                                 GenericToolbox::getFileNameFromFilePath(relative_file_path)
                                 + " (" + GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(absolute_file_path))) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeMod"] = (i_file + 1.) / double(relative_file_path_list.size());

    i_file++;
    absolute_file_path = GenericToolbox::removeRepeatedCharacters(absolute_file_path, "/");
    std::string file_size = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(absolute_file_path)));

    std::string installed_file_path = mod_manager->get_install_mods_base_folder() + "/" + relative_file_path;
    installed_file_path = GenericToolbox::removeRepeatedCharacters(installed_file_path, "/");
    // Check if the installed mod belongs to the selected mod
    if( GenericToolbox::Switch::IO::doFilesAreIdentical( absolute_file_path, installed_file_path ) ){

      // Remove the mod file
      GenericToolbox::deleteFile(installed_file_path);

      // Delete the folder if no other files is present
      std::string empty_folder_path_candidate = GenericToolbox::getFolderPathFromFilePath(installed_file_path);
      while( GenericToolbox::isFolderEmpty( empty_folder_path_candidate ) ) {

        GenericToolbox::deleteEmptyDirectory(empty_folder_path_candidate);

        std::vector<std::string> sub_folder_list = GenericToolbox::splitString(empty_folder_path_candidate, "/");
        if(sub_folder_list.empty()) break; // virtually impossible -> would mean everything has been deleted on the sd
        // decrement folder depth
        empty_folder_path_candidate =
          "/" + GenericToolbox::joinVectorString(
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
void GuiModManager::removeAllMods(bool force_) {

  auto mods_list = GlobalObjects::getModBrowser().getSelector().getSelectionList();

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
      GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeAllMods:current_mod"] =
        mods_list[i_mod] + " (" + std::to_string(i_mod+1) + "/" + std::to_string(mods_list.size()) + ")";
      GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeAllMods"] =  (i_mod + 1.) / double(mods_list.size());
      GuiModManager::removeMod(mods_list[i_mod]);
    }
  }

}
void GuiModManager::apply_mods_list(std::vector<std::string>& modsList_){

  auto* mod_browser = &GlobalObjects::getModBrowser();

  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(modsList_.size());
  for(int i_mod = int(modsList_.size())-1 ; i_mod >= 0 ; i_mod--){
    std::string mod_path = mod_browser->get_current_directory() + "/" + modsList_[i_mod];
    auto mod_files_list = GenericToolbox::getListOfFilesInSubFolders(mod_path);
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
    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::apply_mods_list:current_mod"] =
      modsList_[i_mod] + " (" + std::to_string(i_mod+1) + "/" + std::to_string(modsList_.size()) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::apply_mods_list"] = (i_mod + 1.) / double(modsList_.size());
    mod_browser->getModManager().set_ignored_file_list(mods_ignored_files_list[i_mod]);
    GuiModManager::applyMod(modsList_[i_mod], true); // normally should work without force (tested) but just in case...
    mod_browser->getModManager().get_ignored_file_list().clear();
  }

}
void GuiModManager::checkAllMods() {

  auto* mod_browser = &GlobalObjects::getModBrowser();
  auto modsList = mod_browser->getSelector().getSelectionList();

  mod_browser->getSelector().reset_tags_list();
  auto mods_list = mod_browser->getSelector().getSelectionList();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"] =
      GenericToolbox::getFileNameFromFilePath(mods_list[i_mod]) + " (" + std::to_string(i_mod+1) + "/" + std::to_string(mods_list.size()) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::checkAllMods"] = (i_mod + 1.) / double(mods_list.size());

    // IU update
    std::string result = GuiModManager::getModStatus(mods_list[i_mod]); // actual mod check is here
    // TODO: Change this to a boolean that triggers in the draw loop ?
//    if(GuiGlobals::getCurrentTabModBrowserPtr()){
//      GuiGlobals::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValue(result);
//      NVGcolor color;
//      if(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status_fraction(mods_list[i_mod]) == 0){
//        color.a = 255./255.; color.r = 80./255.; color.g = 80./255.; color.b = 80./255.;
//        GuiGlobals::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          color
//          );
//      }
//      else if(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status_fraction(mods_list[i_mod]) == 1){
//        color.a = 255./255.; color.r = 43./255.; color.g = 81./255.; color.b = 226./255.;
//        GuiGlobals::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          color
//        );
//      }
//      else{
//        GuiGlobals::getCurrentTabModBrowserPtr()->getModsListItems()[mods_list[i_mod]]->setValueActiveColor(
//          {43, 81, 226, 255}
//        );
//      }
//    }

  }

}

// not static
void GuiModManager::startApplyModThread(const std::string& modName_) {
  LogReturnIf(modName_.empty(), "No mod name provided. Can't apply mod.");

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::applyModFunction, this, modName_);
}
void GuiModManager::startRemoveModThread(const std::string& modName_){
  LogReturnIf(modName_.empty(), "No mod name provided. Can't remove mod.");

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::removeModFunction, this, modName_);
}
void GuiModManager::startCheckAllModsThread(){
  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::checkAllModsFunction, this);
}
void GuiModManager::startRemoveAllModsThread(){
  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::removeAllModsFunction, this);
}
void GuiModManager::startApplyModPresetThread(const std::string &modPresetName_){
  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::applyModPresetFunction, this, modPresetName_);
}


bool GuiModManager::applyModFunction(const std::string& modName_){
  // push the progress bar to the view
  _loadingBox_.pushView();

  LogWarning << "Applying: " << modName_ << std::endl;
  if( _loadingBox_.getLoadingView() != nullptr ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setHeader("Applying mod...");
    _loadingBox_.getLoadingView()->setTitlePtr(&modName_);
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyMod"]);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
  }
  GuiModManager::applyMod(modName_, true);

  LogWarning << "Checking: " << modName_ << std::endl;
  if( _loadingBox_.getLoadingView() != nullptr ) {
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setHeader("Checking the applied mod...");
    _loadingBox_.getLoadingView()->setTitlePtr(&modName_);
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"]);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
  }
  GuiModManager::getModStatus(modName_);

  _loadingBox_.popView();

  _loadingBox_.getLoadingView()->reset();
  return true;
}
bool GuiModManager::applyModPresetFunction(const std::string& presetName_){
  // push the progress bar to the view
  _loadingBox_.pushView();

  LogInfo << "Removing all installed mods..." << std::endl;
  if( _loadingBox_.getLoadingView() != nullptr ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    _loadingBox_.getLoadingView()->setHeader("Removing all installed mods...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeAllMods"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeMod"]);
  }
  GuiModManager::removeAllMods(true);

  LogInfo("Applying Mod Preset");
  if( _loadingBox_.getLoadingView() ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setHeader("Applying mod preset...");
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xff, 0xc8));
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::apply_mods_list:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyMod"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);
  }
  auto modsList = GlobalObjects::getModBrowser().get_mods_preseter().get_mods_list(presetName_);
  GuiModManager::apply_mods_list(modsList);

  LogInfo("Checking all mods status...");
  if( _loadingBox_.getLoadingView() ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setHeader("Checking all mods status...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::checkAllMods();

  _loadingBox_.getLoadingView()->reset();
  _loadingBox_.popView();
  brls::Application::unblockInputs();
  return true;
}
bool GuiModManager::removeModFunction(const std::string& modName_){
  // push the progress bar to the view
  _loadingBox_.pushView();

  LogWarning << "Removing: " << modName_ << std::endl;
  if( _loadingBox_.getLoadingView() != nullptr ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setHeader("Removing mod...");
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    _loadingBox_.getLoadingView()->setTitlePtr(&modName_);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeMod"]);
  }
  GuiModManager::removeMod(modName_);

  LogWarning << "Checking: " << modName_ << std::endl;
  if( _loadingBox_.getLoadingView() != nullptr ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setHeader("Checking mod...");
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setTitlePtr(&modName_);
    _loadingBox_.getLoadingView()->setSubTitlePtr(
        &GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(
        &GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(
        &GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::getModStatus(modName_);

  brls::Application::unblockInputs();

  _loadingBox_.popView();

  _loadingBox_.getLoadingView()->reset();
  return true;
}
bool GuiModManager::checkAllModsFunction(){
  // push the progress bar to the view
  _loadingBox_.pushView();

  LogInfo("Checking all mods status...");
  if( _loadingBox_.getLoadingView() != nullptr ) {
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setHeader("Checking all mods status...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"]);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::checkAllMods();
  LogInfo << "Check all mods done." << std::endl;

  _loadingBox_.getLoadingView()->reset();

  _loadingBox_.popView();
  brls::Application::unblockInputs();
  return true;
}
bool GuiModManager::removeAllModsFunction(){
  // push the progress bar to the view
  _loadingBox_.pushView();

  LogInfo("Removing all installed mods...");
  if( _loadingBox_.getLoadingView() != nullptr ) {
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0xff, 0x64, 0x64));
    _loadingBox_.getLoadingView()->setHeader("Removing all installed mods...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeAllMods"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeMod"]);
  }
  GuiModManager::removeAllMods(true);

  LogInfo("Checking all mods status...");
  if( _loadingBox_.getLoadingView() != nullptr ) {
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setHeader("Checking all mods status...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::getModStatus:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::getModStatus"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::checkAllMods();

  _loadingBox_.getLoadingView()->reset();
  _loadingBox_.popView();
  brls::Application::unblockInputs();
  return true;
}

