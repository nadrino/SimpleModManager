//
// Created by Adrien BLANCHET on 28/06/2020.
//

#include "GuiModManager.h"

#include "GenericToolbox.Switch.h"
#include "GenericToolbox.Vector.h"
#include "GenericToolbox.Misc.h"
#include "Logger.h"

#include <string>
#include <vector>
#include <future>
#include <chrono>
#include <algorithm>
#include <switch.h>


LoggerInit([]{
  Logger::setUserHeaderStr("[GuiModManager]");
});

namespace {

constexpr long long kDeleteModFolderCooldownMs = 750;
constexpr long long kDeleteModFolderFsSettleNs = 250000000; // 250ms

long long monotonicMs() {
  const auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

bool safeIsDir(const std::string& path_) {
  try { return GenericToolbox::isDir(path_); }
  catch(...) { return false; }
}

bool safeIsFile(const std::string& path_) {
  try { return GenericToolbox::isFile(path_); }
  catch(...) { return false; }
}

bool safeIsDirEmpty(const std::string& path_) {
  try { return GenericToolbox::isDirEmpty(path_); }
  catch(...) { return false; }
}

bool safeRmFile(const std::string& path_) {
  for( int attempt = 0; attempt < 10; ++attempt ) {
    if( !safeIsFile(path_) ) {
      return true;
    }
    try {
      if( GenericToolbox::rm(path_) ) {
        return true;
      }
    }
    catch(...) {}
    svcSleepThread(50000000); // 50ms
  }
  return !safeIsFile(path_);
}

bool safeRmDir(const std::string& path_) {
  for( int attempt = 0; attempt < 10; ++attempt ) {
    if( !safeIsDir(path_) ) {
      return true;
    }
    try {
      if( GenericToolbox::rmDir(path_) ) {
        return true;
      }
    }
    catch(...) {}
    svcSleepThread(60000000); // 60ms
  }
  return !safeIsDir(path_);
}

void bumpDeleteProgress(double& progress_, size_t processedEntries_) {
  const double p = 1.0 - (1.0 / double(processedEntries_ + 3));
  progress_ = std::min(0.97, p);
}

bool deleteDirectoryTreeForSd(
    const std::string& rootPath_,
    bool& cancelRequested_,
    std::string& currentEntry_,
    double& progress_ ) {
  if( !safeIsDir(rootPath_) ) {
    progress_ = 1;
    return true;
  }

  bool success = true;
  size_t processedEntries = 0;
  std::vector<std::string> dirsToRemove;
  std::vector<std::string> dirsToVisit{ rootPath_ };

  while( !dirsToVisit.empty() ) {
    if( cancelRequested_ ) {
      return false;
    }

    const std::string dir = dirsToVisit.back();
    dirsToVisit.pop_back();
    dirsToRemove.push_back(dir);

    std::vector<std::string> entries;
    try {
      entries = GenericToolbox::ls(dir);
    }
    catch(...) {
      success = false;
      continue;
    }

    for( const auto& entry : entries ) {
      if( cancelRequested_ ) {
        return false;
      }
      if( entry == "." || entry == ".." ) {
        continue;
      }

      const std::string childPath = GenericToolbox::joinPath(dir, entry);
      currentEntry_ = entry;

      if( safeIsDir(childPath) ) {
        dirsToVisit.push_back(childPath);
      }
      else if( safeIsFile(childPath) ) {
        if( !safeRmFile(childPath) ) {
          success = false;
        }
        processedEntries++;
        bumpDeleteProgress(progress_, processedEntries);
      }
    }
  }

  for( auto it = dirsToRemove.rbegin(); it != dirsToRemove.rend(); ++it ) {
    if( cancelRequested_ ) {
      return false;
    }
    currentEntry_ = GenericToolbox::getFileName(*it);
    if( !safeRmDir(*it) ) {
      success = false;
    }
    processedEntries++;
    bumpDeleteProgress(progress_, processedEntries);
  }

  progress_ = success ? 1 : progress_;
  return success && !safeIsDir(rootPath_);
}

} // namespace


void GuiModManager::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus) {
  _triggerUpdateModsDisplayedStatus_ = triggerUpdateModsDisplayedStatus;
}

void GuiModManager::setTriggerRebuildModBrowser(bool triggerRebuildModBrowser) {
  _triggerRebuildModBrowser_ = triggerRebuildModBrowser;
}

bool GuiModManager::isTriggerUpdateModsDisplayedStatus() const {
  return _triggerUpdateModsDisplayedStatus_;
}
bool GuiModManager::isTriggerRebuildModBrowser() const {
  return _triggerRebuildModBrowser_;
}
bool GuiModManager::isBackgroundTaskRunning() const {
  if( not _asyncResponse_.valid() ){
    return false;
  }
  return _asyncResponse_.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}
const GameBrowser &GuiModManager::getGameBrowser() const { return _gameBrowser_; }
GameBrowser &GuiModManager::getGameBrowser(){ return _gameBrowser_; }

void GuiModManager::applyMod(const std::string &modName_) {
  LogWarning << __METHOD_NAME__ << ": " << modName_ << std::endl;
  modApplyMonitor = ModApplyMonitor();

  std::string modPath = GenericToolbox::joinPath(_gameBrowser_.getModManager().getGameFolderPath(), modName_);
  LogInfo << "Installing files in: " << modPath << std::endl;

  std::vector<std::string> modFilesList = GenericToolbox::lsFilesRecursive(modPath);

  // deleting ignored entries
  for(int i_mod = int(modFilesList.size()) - 1 ; i_mod >= 0 ; i_mod--){
    if(GenericToolbox::doesElementIsInVector(
        modFilesList[i_mod], _gameBrowser_.getModManager().getIgnoredFileList())){
      modFilesList.erase(modFilesList.begin() + i_mod);
    }
  }
  LogInfo << modFilesList.size() << " files to be installed." << std::endl;

  for(size_t iFile = 0 ; iFile < modFilesList.size() ; iFile++){
    LogReturnIf( _triggeredOnCancel_, "Cancel detected. Leaving " << __METHOD_NAME__ );

    if( GenericToolbox::getFileName(modFilesList[iFile])[0] == '.' ){
      // ignoring cached files
      continue;
    }

    std::string filePath = GenericToolbox::joinPath(modPath, modFilesList[iFile]);
    std::string fileSizeStr = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(filePath)));

    modApplyMonitor.currentFile = GenericToolbox::getFileName(modFilesList[iFile]) + " (" + fileSizeStr + ")";
    modApplyMonitor.progress = double(iFile + 1) / double(modFilesList.size());

    std::string installPath = GenericToolbox::joinPath(_gameBrowser_.getModManager().fetchCurrentPreset().installBaseFolder, modFilesList[iFile] );
    GenericToolbox::Switch::IO::copyFile(filePath, installPath);
  }
  _gameBrowser_.getModManager().resetModCache(modName_);

}
void GuiModManager::getModStatus(const std::string &modName_, bool useCache_) {
  LogWarning << __METHOD_NAME__ << ": " << modName_ << ", with cache? " << useCache_ << std::endl;
  modCheckMonitor = ModCheckMonitor();

  auto& modManager = _gameBrowser_.getModManager();
  const bool forceRecheck = !useCache_;
  modCheckMonitor.currentFile = forceRecheck ? "Checking changed files..." : "Refreshing status cache...";
  modCheckMonitor.progress = 0.5;

  auto result = modManager.refreshModStatus(modName_, forceRecheck);
  if( result == ResultModAction::Fail ){
    LogWarning << "Could not refresh mod status: " << modName_ << std::endl;
  }

  int modIndex = modManager.getModIndex(modName_);
  if( modIndex != -1 ){
    const std::string configPresetName{modManager.fetchCurrentPreset().name};
    LogInfo << modName_ << " -> " << modManager.getModList()[modIndex].getStatus(configPresetName) << std::endl;
  }
  modCheckMonitor.progress = 1;
}
void GuiModManager::removeMod(const std::string &modName_){
  this->removeModInstalledFiles(modName_, false);
}
void GuiModManager::removeModInstalledFiles(const std::string &modName_, bool forceUnknownInstalledFiles_){
  LogWarning << __METHOD_NAME__ << ": " << modName_ << std::endl;
  modRemoveMonitor = ModRemoveMonitor();

  auto& modManager = _gameBrowser_.getModManager();
  std::string modPath = GenericToolbox::joinPath( modManager.getGameFolderPath(), modName_ );
  auto modFileList = GenericToolbox::lsFilesRecursive(modPath);

  int iFile{0};
  for(auto &modFile : modFileList){
    LogReturnIf( _triggeredOnCancel_, "Cancel detected. Leaving " << __METHOD_NAME__ );

    const auto modFileName = GenericToolbox::getFileName(modFile);
    if( modFileName.empty() || modFileName[0] == '.' ){
      continue;
    }

    modRemoveMonitor.currentFile = modFileName;
    modRemoveMonitor.currentFile += " (" + GenericToolbox::joinAsString("/", iFile+1, modFileList.size()) + ")";
    modRemoveMonitor.progress = (iFile++ + 1.) / double(modFileList.size());

    std::string srcFilePath = GenericToolbox::joinPath( modPath, modFile );
    std::string dstFilePath = GenericToolbox::joinPath(modManager.fetchCurrentPreset().installBaseFolder, modFile );

    // Check if the installed mod belongs to the selected mod
    bool shouldRemoveInstalledFile = false;
    try {
      shouldRemoveInstalledFile = GenericToolbox::Switch::IO::doFilesAreIdentical(srcFilePath, dstFilePath);
    }
    catch(...) {}

    if( !shouldRemoveInstalledFile && forceUnknownInstalledFiles_ && safeIsFile(dstFilePath) ){
      bool installedFileBelongsToAnotherMod = false;
      const auto& modList = modManager.getModList();
      for( const auto& otherMod : modList ){
        if( otherMod.modName == modName_ ){
          continue;
        }

        const std::string otherModFilePath = GenericToolbox::joinPath(
            GenericToolbox::joinPath(modManager.getGameFolderPath(), otherMod.modName),
            modFile);

        if( !safeIsFile(otherModFilePath) ){
          continue;
        }

        try {
          if( GenericToolbox::Switch::IO::doFilesAreIdentical(otherModFilePath, dstFilePath) ){
            installedFileBelongsToAnotherMod = true;
            break;
          }
        }
        catch(...) {}
      }

      shouldRemoveInstalledFile = !installedFileBelongsToAnotherMod;
    }

    if( shouldRemoveInstalledFile ){

      // Remove the mod file with multiple retries
      if( !safeRmFile(dstFilePath) ){
        LogError << "Could not remove installed mod file: " << dstFilePath << std::endl;
        continue;
      }

      // Delete the folder if no other files is present
      std::string emptyFolderCandidate = GenericToolbox::getFolderPath( dstFilePath );
      int safetyCounter = 0;
      while( safeIsDirEmpty( emptyFolderCandidate ) ) {

        // Safety check to prevent deleting system folders
        std::string installBase = modManager.fetchCurrentPreset().installBaseFolder;
        if( emptyFolderCandidate.find(installBase) != 0 ) break;
        if( emptyFolderCandidate == installBase ) break;

        // Safety counter to prevent infinite loops
        if( safetyCounter++ > 50 ) break;

        // Delete directory with retries
        if( !safeRmDir(emptyFolderCandidate) ){
          goto folder_cleanup_done;
        }

        // Longer delay to prevent filesystem issues
        svcSleepThread(20000000); // 20ms delay

        emptyFolderCandidate = GenericToolbox::getFolderPath(emptyFolderCandidate);
      }
      folder_cleanup_done:;
    }

    // Small delay between file deletions to prevent filesystem overload
    svcSleepThread(10000000); // 10ms delay
  }

  _gameBrowser_.getModManager().resetModCache(modName_);

}
bool GuiModManager::deleteModFolderFromSd(const std::string &modName_) {
  LogWarning << __METHOD_NAME__ << ": " << modName_ << std::endl;
  modDeleteFolderMonitor = ModDeleteFolderMonitor();
  modDeleteFolderMonitor.currentEntry = "Preparing delete...";

  auto& modManager = _gameBrowser_.getModManager();
  const std::string modFolderPath = GenericToolbox::joinPath(modManager.getGameFolderPath(), modName_);

  const bool deleted = deleteDirectoryTreeForSd(
      modFolderPath,
      _triggeredOnCancel_,
      modDeleteFolderMonitor.currentEntry,
      modDeleteFolderMonitor.progress );

  bool stillPresentInModList = true;
  for( int attempt = 0; attempt < 10; ++attempt ) {
    try {
      modManager.updateModList();
      const auto& mods = modManager.getModList();
      stillPresentInModList = std::any_of(mods.begin(), mods.end(),
          [&](const auto& entry){ return entry.modName == modName_; });
      if( !stillPresentInModList ) {
        break;
      }
    }
    catch(...) {
      stillPresentInModList = safeIsDir(modFolderPath);
    }
    svcSleepThread(120000000); // 120ms
  }

  if( deleted && !stillPresentInModList ) {
    modDeleteFolderMonitor.currentEntry = "Deleted.";
    modDeleteFolderMonitor.progress = 1;
    LogInfo << "Deleted mod folder: " << modFolderPath << std::endl;
    return true;
  }

  modDeleteFolderMonitor.currentEntry = "Delete failed.";
  LogError << "Failed to delete mod folder: " << modFolderPath << " -> "
           << (deleted ? "folder still exists" : "recursive delete failed") << std::endl;
  return false;
}
void GuiModManager::removeAllMods() {
  LogWarning << __METHOD_NAME__ << std::endl;
  modRemoveAllMonitor = ModRemoveAllMonitor();

  auto modList = _gameBrowser_.getModManager().getModList();

  for(int iMod = 0 ; iMod < int(modList.size()) ; iMod++ ){
    LogReturnIf( _triggeredOnCancel_, "Cancel detected. Leaving " << __METHOD_NAME__ );

    modRemoveAllMonitor.currentMod = modList[iMod].modName + " (" + std::to_string(iMod + 1) + "/" + std::to_string(modList.size()) + ")";
    modRemoveAllMonitor.progress = (iMod + 1.) / double(modList.size());

    this->removeMod( modList[iMod].modName );
  }

}
void GuiModManager::applyModsList(std::vector<std::string>& modsList_){
  LogWarning << __METHOD_NAME__ << ": " << GenericToolbox::toString(modsList_) << std::endl;
  modApplyListMonitor = ModApplyListMonitor();

  // checking for overwritten files in advance:

  // All the files that will be installed are saved here
  std::vector<std::string> appliedFileList;

  // This is the filter for override files
  std::vector<std::vector<std::string>> ignoredFileListPerMod( modsList_.size() ); // ignoredFileListPerMod[iMod][iIgnoredFile];

  // looping backward as the last mods will be the ones for which their files are going to be kept
  for( int iMod = int(modsList_.size()) - 1 ; iMod >= 0 ; iMod-- ){
    std::string modPath = GenericToolbox::joinPath( _gameBrowser_.getModManager().getGameFolderPath(), modsList_[iMod] );
    auto fileList = GenericToolbox::lsFilesRecursive(modPath);
    for(auto& file : fileList){
      if( GenericToolbox::doesElementIsInVector(file, appliedFileList) ){
        // this file will be installed by a latter mod
        ignoredFileListPerMod[iMod].emplace_back(file);
      }
      else{
        // new file, add it to the list
        appliedFileList.emplace_back(file);
      }
    }
  }

  // applying mods with ignored files
  for( size_t iMod = 0 ; iMod < modsList_.size() ; iMod++ ){
    LogReturnIf( _triggeredOnCancel_, "Cancel detected. Leaving " << __METHOD_NAME__ );

    modApplyListMonitor.currentMod = modsList_[iMod];
    modApplyListMonitor.currentMod += " (" + std::to_string(iMod + 1) + "/" + std::to_string(modsList_.size()) + ")";
    modApplyListMonitor.progress = (double(iMod) + 1.) / double(modsList_.size());

    _gameBrowser_.getModManager().setIgnoredFileList(ignoredFileListPerMod[iMod]);
    this->applyMod( modsList_[iMod] );
    _gameBrowser_.getModManager().getIgnoredFileList().clear();

  }

}
void GuiModManager::checkAllMods(bool useCache_) {
  LogWarning << __METHOD_NAME__ << ": with cache? " << useCache_ << std::endl;
  modCheckAllMonitor = ModCheckAllMonitor();

  auto modList = _gameBrowser_.getModManager().getModList();
  for( size_t iMod = 0 ; iMod < modList.size() ; iMod++ ){
    LogReturnIf( _triggeredOnCancel_, "Cancel detected. Leaving " << __METHOD_NAME__ );

    // progress
    modCheckAllMonitor.currentMod = modList[iMod].modName;
    modCheckAllMonitor.currentMod += " (" + std::to_string(iMod + 1) + "/" + std::to_string(modList.size()) + ")";
    modCheckAllMonitor.progress = (double(iMod) + 1.) / double(modList.size());

    this->getModStatus( modList[iMod].modName, useCache_ );
  }
}

void GuiModManager::startApplyModThread(const std::string& modName_) {
  LogReturnIf(modName_.empty(), "No mod name provided. Can't apply mod.");

  this->_triggeredOnCancel_ = false;

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::applyModFunction, this, modName_);
}
void GuiModManager::startRemoveModThread(const std::string& modName_){
  LogReturnIf(modName_.empty(), "No mod name provided. Can't remove mod.");

  this->_triggeredOnCancel_ = false;

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::removeModFunction, this, modName_);
}
void GuiModManager::startDeleteModFolderThread(const std::string& modName_){
  LogReturnIf(modName_.empty(), "No mod name provided. Can't delete mod folder.");
  if( _triggerRebuildModBrowser_ ){
    brls::Application::notify("Refreshing mod list. Please wait.");
    return;
  }
  if( this->isBackgroundTaskRunning() ){
    brls::Application::notify("A mod task is already running.");
    return;
  }
  if( _deleteModFolderRunning_.load() ){
    brls::Application::notify("A mod delete is already finishing.");
    return;
  }

  const long long lastDeleteFinished = _lastDeleteModFolderFinishedMs_.load();
  if( lastDeleteFinished > 0 && monotonicMs() - lastDeleteFinished < kDeleteModFolderCooldownMs ){
    brls::Application::notify("Please wait before deleting another mod.");
    return;
  }

  this->_triggeredOnCancel_ = false;
  this->_triggerRebuildModBrowser_ = false;
  _deleteModFolderRunning_.store(true);

  _asyncResponse_ = std::async(std::launch::async, &GuiModManager::deleteModFolderFunction, this, modName_);
}
void GuiModManager::startCheckAllModsThread(){
  if( this->isBackgroundTaskRunning() ){
    return;
  }
  this->_triggeredOnCancel_ = false;

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::checkAllModsFunction, this);
}
void GuiModManager::startRemoveAllModsThread(){
  this->_triggeredOnCancel_ = false;

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::removeAllModsFunction, this);
}
void GuiModManager::startApplyModPresetThread(const std::string &modPresetName_){
  this->_triggeredOnCancel_ = false;

  // start the parallel thread
  _asyncResponse_ = std::async(&GuiModManager::applyModPresetFunction, this, modPresetName_);
}

bool GuiModManager::applyModFunction(const std::string& modName_){
  // push the progress bar to the view
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  LogWarning << "Applying: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setHeaderTitle("Applying mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::greenNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modName_ );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modApplyMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modApplyMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &GenericToolbox::Switch::Utils::b.progressMap["copyFile"] );
  this->applyMod( modName_ );
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  LogWarning << "Checking: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking the applied mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->getModStatus( modName_, false );

  return this->leaveModAction(true);
}
bool GuiModManager::applyModPresetFunction(const std::string& presetName_){
  // push the progress bar to the view
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  LogInfo << "Removing all installed mods..." << std::endl;
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::redNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing all installed mods...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modRemoveAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &modRemoveMonitor.progress );
  this->removeAllMods();
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  LogInfo("Applying Mod Preset");
  _loadingPopup_.getMonitorView()->setHeaderTitle("Applying mod preset...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::greenNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modApplyListMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modApplyMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modApplyListMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);

  std::vector<std::string> modsList;
  for( auto& preset : _gameBrowser_.getModPresetHandler().getPresetList() ){
    if( preset.name == presetName_ ){ modsList = preset.modList; break; }
  }
  this->applyModsList(modsList);
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking all mods status...");

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->checkAllMods();
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  return this->leaveModAction(true);
}
bool GuiModManager::removeModFunction(const std::string& modName_){
  // push the progress bar to the view
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  LogWarning << "Removing: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::redNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"] );
  this->removeMod( modName_ );
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  LogWarning << "Checking: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setProgressColor( GenericToolbox::Borealis::blueNvgColor );
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking mod...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->getModStatus(modName_);

  return this->leaveModAction(true);
}
bool GuiModManager::deleteModFolderFunction(const std::string& modName_){
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  bool success = false;
  try {
    LogWarning << "Removing installed files before deleting mod folder: " << modName_ << std::endl;
    _loadingPopup_.getMonitorView()->setHeaderTitle("Removing installed mod...");
    _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::redNvgColor);
    _loadingPopup_.getMonitorView()->resetMonitorAddresses();
    _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
    _loadingPopup_.getMonitorView()->setSubTitlePtr(&modRemoveMonitor.currentFile);
    _loadingPopup_.getMonitorView()->setProgressFractionPtr(&modRemoveMonitor.progress);
    _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);

    this->removeModInstalledFiles(modName_, true);
    if( _triggeredOnCancel_ ){
      _triggerRebuildModBrowser_ = true;
      this->finishDeleteModFolderTask();
      return this->leaveModAction(false);
    }

    svcSleepThread(kDeleteModFolderFsSettleNs);

    LogWarning << "Deleting mod folder: " << modName_ << std::endl;
    _loadingPopup_.getMonitorView()->setHeaderTitle("Deleting mod folder...");
    _loadingPopup_.getMonitorView()->resetMonitorAddresses();
    _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
    _loadingPopup_.getMonitorView()->setSubTitlePtr(&modDeleteFolderMonitor.currentEntry);
    _loadingPopup_.getMonitorView()->setProgressFractionPtr(&modDeleteFolderMonitor.progress);

    success = this->deleteModFolderFromSd(modName_);
  }
  catch(...) {
    LogError << "Unexpected exception while deleting mod folder: " << modName_ << std::endl;
    modDeleteFolderMonitor.currentEntry = "Delete failed.";
    modDeleteFolderMonitor.progress = 0;
    success = false;
  }

  svcSleepThread(kDeleteModFolderFsSettleNs);
  _triggerRebuildModBrowser_ = true;
  this->finishDeleteModFolderTask();

  return this->leaveModAction(success);
}

void GuiModManager::finishDeleteModFolderTask(){
  _lastDeleteModFolderFinishedMs_.store(monotonicMs());
  _deleteModFolderRunning_.store(false);
}

bool GuiModManager::checkAllModsFunction(){
  // push the progress bar to the view
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  LogInfo("Resetting mods cache before recheck...");
  _gameBrowser_.getModManager().resetAllModsCacheAndFile();

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Rechecking all mods status...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->checkAllMods();
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }
  LogInfo << "Check all mods done." << std::endl;

  return this->leaveModAction(true);
}
bool GuiModManager::removeAllModsFunction(){
  // push the progress bar to the view
  _loadingPopup_.pushView();
  _loadingPopup_.getMonitorView()->setExecOnDelete([this]{ this->_triggeredOnCancel_ = true; });

  LogInfo("Removing all installed mods...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::redNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing all installed mods...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modRemoveAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &modRemoveMonitor.progress );
  this->removeAllMods();
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking all mods status...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"] );
  this->checkAllMods();
  if( _triggeredOnCancel_ ){ return this->leaveModAction(false); }

  return this->leaveModAction(true);
}


bool GuiModManager::leaveModAction(bool isSuccess_){
  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();
  LogInfo << "Leaving mod action with success? " << isSuccess_ << std::endl;
  return isSuccess_;
}
