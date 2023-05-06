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


const GameBrowser &GuiModManager::getGameBrowser() const { return _gameBrowser_; }
GameBrowser &GuiModManager::getGameBrowser(){ return _gameBrowser_; }

// static
void GuiModManager::applyMod(const std::string &modName_, bool force_) {

  std::string modPath = GenericToolbox::joinPath(_gameBrowser_.getModManager().getGameFolderPath(), modName_);
  LogInfo << "Installing files in: " << modPath << std::endl;

  std::vector<std::string> modFilesList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  // deleting ignored entries
  for(int i_mod = int(modFilesList.size()) - 1 ; i_mod >= 0 ; i_mod--){
    if(GenericToolbox::doesElementIsInVector(
        modFilesList[i_mod], _gameBrowser_.getModManager().getIgnoredFileList())){
      modFilesList.erase(modFilesList.begin() + i_mod);
    }
  }
  LogInfo << modFilesList.size() << " files to be installed." << std::endl;

  std::string replace_option;
  if(force_) replace_option = "Yes to all";
  bool is_conflict;
  std::stringstream ss_files_list;

  LogDebug << GenericToolbox::parseVectorAsString(modFilesList) << std::endl;

  for(size_t iFile = 0 ; iFile < modFilesList.size() ; iFile++){

    if(modFilesList[iFile][0] == '.'){
      // ignoring cached files
      continue;
    }

    std::string filePath = GenericToolbox::joinPath(modPath, modFilesList[iFile]);
    std::string fileSizeStr = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(filePath)));

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyMod:current_file"] = GenericToolbox::getFileNameFromFilePath(modFilesList[iFile]) + " (" + fileSizeStr + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyMod"] = double(iFile + 1) / double(modFilesList.size());

    std::string installPath = GenericToolbox::joinPath(_gameBrowser_.getModManager().fetchCurrentPreset().installBaseFolder, modFilesList[iFile] );
    GenericToolbox::doesPathIsFile(installPath) ? is_conflict = true : is_conflict = false;
    if(not is_conflict or replace_option == "Yes to all" or replace_option == "Yes"){
      GenericToolbox::Switch::IO::copyFile(filePath, installPath);
    }

  }
  _gameBrowser_.getModManager().resetModCache(modName_);

}
void GuiModManager::getModStatus(const std::string &modName_, bool useCache_) {
  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE
  std::string result;

  auto& modManager = _gameBrowser_.getModManager();
  int modIndex = modManager.getModIndex( modName_ );

  // entry valid?
  if( modIndex == -1 ){ return; }

  // cached?
  std::string configPresetName{modManager.fetchCurrentPreset().name};
  if( useCache_ and GenericToolbox::doesKeyIsInMap(configPresetName, modManager.getModList()[modIndex].applyCache ) ){
    LogDebug << configPresetName << ":" << modManager.getModList()[modIndex].modName << " CACHED: " << modManager.getModList()[modIndex].applyCache[configPresetName].statusStr << std::endl;
    return;
  }

  // recheck?
  auto& cacheEntry = modManager.getModList()[modIndex].applyCache[configPresetName];
  std::string modPath = GenericToolbox::joinPath(modManager.getGameFolderPath(), modName_ );

  int sameFileCount = 0;

  GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"] = "Listing mod files...";
  std::vector<std::string> modFileList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  for( size_t iFile = 0 ; iFile < modFileList.size() ; iFile++ ){

    std::string srcFilePath = GenericToolbox::joinPath(modPath, modFileList[iFile] );
    std::string dstFilePath = GenericToolbox::joinPath(modManager.fetchCurrentPreset().installBaseFolder, modFileList[iFile] );

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"] = GenericToolbox::getFileNameFromFilePath(modFileList[iFile]);
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"] = (iFile + 1.) / double(modFileList.size());

    if(GenericToolbox::Switch::IO::doFilesAreIdentical(
        modManager.fetchCurrentPreset().installBaseFolder + "/" + modFileList[iFile],
        srcFilePath
    )){ sameFileCount++; }

  }

  cacheEntry.applyFraction = double(sameFileCount) / double(modFileList.size());

  if     ( modFileList.empty()           ) cacheEntry.statusStr = "NO FILE";
  else if( cacheEntry.applyFraction == 0 ) cacheEntry.statusStr = "INACTIVE";
  else if( cacheEntry.applyFraction == 1 ) cacheEntry.statusStr = "ACTIVE";
  else cacheEntry.statusStr = "PARTIAL (" + GenericToolbox::joinAsString("/", sameFileCount, modFileList.size()) + ")";

  LogInfo << modName_ << " -> " << cacheEntry.statusStr << std::endl;
  modManager.dumpModStatusCache();
}
void GuiModManager::removeMod(const std::string &modName_){
  LogDebug << __METHOD_NAME__ << std::endl;

  std::string modPath = GenericToolbox::joinPath( _gameBrowser_.getModManager().getGameFolderPath(), modName_ );
  auto modFileList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  int iFile{0};
  Toolbox::reset_last_displayed_value();
  for(auto &modFile : modFileList){

    std::string srcFilePath = GenericToolbox::joinPath(modPath, modFile );
    auto fileSize = double(GenericToolbox::getFileSize(srcFilePath));

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeMod:current_file"] =
        GenericToolbox::getFileNameFromFilePath(modFile) + " (" + GenericToolbox::parseSizeUnits(fileSize) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeMod"] = (iFile++ + 1.) / double(modFileList.size());

    std::string dstFilePath = GenericToolbox::joinPath(_gameBrowser_.getModManager().fetchCurrentPreset().installBaseFolder, modFile );
    // Check if the installed mod belongs to the selected mod
    if( GenericToolbox::Switch::IO::doFilesAreIdentical(srcFilePath, dstFilePath ) ){

      // Remove the mod file
      GenericToolbox::deleteFile( dstFilePath );

      // Delete the folder if no other files is present
      std::string emptyFolderCandidate = GenericToolbox::getFolderPathFromFilePath( dstFilePath );
      while( GenericToolbox::isFolderEmpty( emptyFolderCandidate ) ) {

        GenericToolbox::deleteEmptyDirectory( emptyFolderCandidate );

        std::vector<std::string> subFolderList = GenericToolbox::splitString(emptyFolderCandidate, "/");
        if(subFolderList.empty()) break; // virtually impossible -> would mean everything has been deleted on the sd
        // decrement folder depth
        emptyFolderCandidate = "/" + GenericToolbox::joinVectorString( subFolderList, "/", 0, int(subFolderList.size()) - 1 );
      }
    }
  }

  _gameBrowser_.getModManager().resetModCache(modName_);

}
void GuiModManager::removeAllMods(bool force_) {

  auto modList = _gameBrowser_.getModManager().getModList();
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
    for( int i_mod = 0 ; i_mod < int(modList.size()) ; i_mod++ ){
      GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::removeAllMods:current_mod"] =
          modList[i_mod].modName + " (" + std::to_string(i_mod+1) + "/" + std::to_string(modList.size()) + ")";
      GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::removeAllMods"] =  (i_mod + 1.) / double(modList.size());
      GuiModManager::removeMod( modList[i_mod].modName );
    }
  }

}
void GuiModManager::applyModsList(std::vector<std::string>& modsList_){


  // checking for overwritten files in advance
  std::vector<std::string> applied_files_listing;
  std::vector<std::vector<std::string>> mods_ignored_files_list(modsList_.size());
  for(int i_mod = int(modsList_.size())-1 ; i_mod >= 0 ; i_mod--){
    std::string modPath = GenericToolbox::joinPath( _gameBrowser_.getModManager().getGameFolderPath(), modsList_[i_mod] );
    auto fileList = GenericToolbox::getListOfFilesInSubFolders(modPath);
    for(auto& file : fileList){
      if(GenericToolbox::doesElementIsInVector(file, applied_files_listing)){
        mods_ignored_files_list[i_mod].emplace_back(file);
      }
      else {
        applied_files_listing.emplace_back(file);
      }
    }
  }

  // applying mods with ignored files
  for(int i_mod = 0 ; i_mod < int(modsList_.size()) ; i_mod++){
    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyModsList:current_mod"] =
      modsList_[i_mod] + " (" + std::to_string(i_mod+1) + "/" + std::to_string(modsList_.size()) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyModsList"] = (i_mod + 1.) / double(modsList_.size());
    _gameBrowser_.getModManager().setIgnoredFileList(mods_ignored_files_list[i_mod]);
    GuiModManager::applyMod(modsList_[i_mod], true); // normally should work without force (tested) but just in case...
    _gameBrowser_.getModManager().getIgnoredFileList().clear();
  }

}
void GuiModManager::checkAllMods(bool useCache_) {

  auto modList = _gameBrowser_.getModManager().getModList();
  for(int i_mod = 0 ; i_mod < int(modList.size()) ; i_mod++){

    GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"] =
        modList[i_mod].modName + " (" + std::to_string(i_mod + 1) + "/" + std::to_string(modList.size()) + ")";
    GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::checkAllMods"] = (i_mod + 1.) / double(modList.size());

    // IU update
    GuiModManager::getModStatus(modList[i_mod].modName, useCache_); // actual mod check is here
    _triggerUpdateModsDisplayedStatus_ = true;
  }

  _triggerUpdateModsDisplayedStatus_ = true;
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
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"]);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
  }
  GuiModManager::getModStatus(modName_, false);
  _triggerUpdateModsDisplayedStatus_ = true;

  _loadingBox_.getLoadingView()->reset();
  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingBox_.popView();
  brls::Application::unblockInputs();
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
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyModsList:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::applyMod:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::applyMod"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);
  }

  std::vector<std::string> modsList;
  for( auto& preset : _gameBrowser_.getModPresetHandler().getPresetList() ){
    if( preset.name == presetName_ ){
      modsList = preset.modList;
      break;
    }
  }
  this->applyModsList(modsList);

  LogInfo("Checking all mods status...");
  if( _loadingBox_.getLoadingView() ){
    _loadingBox_.getLoadingView()->reset();
    _loadingBox_.getLoadingView()->setProgressColor(nvgRGB(0x00, 0xc8, 0xff));
    _loadingBox_.getLoadingView()->setHeader("Checking all mods status...");
    _loadingBox_.getLoadingView()->setTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::checkAllMods:current_mod"]);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::checkAllMods();

  _loadingBox_.getLoadingView()->reset();
  _triggerUpdateModsDisplayedStatus_ = true;
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
        &GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(
        &GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(
        &GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::getModStatus(modName_);
  _triggerUpdateModsDisplayedStatus_ = true;

  _loadingBox_.getLoadingView()->reset();
  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingBox_.popView();
  brls::Application::unblockInputs();
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
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"]);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  this->checkAllMods();
  LogInfo << "Check all mods done." << std::endl;

  _loadingBox_.getLoadingView()->reset();
  _triggerUpdateModsDisplayedStatus_ = true;
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
    _loadingBox_.getLoadingView()->setSubTitlePtr(&GenericToolbox::Switch::Utils::b.strMap["ext_mod_manager::generateStatusStr:current_file"]);
    _loadingBox_.getLoadingView()->setProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["ext_mod_manager::generateStatusStr"]);
    _loadingBox_.getLoadingView()->setEnableSubLoadingBar(true);
    _loadingBox_.getLoadingView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  }
  GuiModManager::checkAllMods();

  _loadingBox_.getLoadingView()->reset();
  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingBox_.popView();
  brls::Application::unblockInputs();
  return true;
}

void GuiModManager::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus) {
  _triggerUpdateModsDisplayedStatus_ = triggerUpdateModsDisplayedStatus;
}

bool GuiModManager::isTriggerUpdateModsDisplayedStatus() const {
  return _triggerUpdateModsDisplayedStatus_;
}


