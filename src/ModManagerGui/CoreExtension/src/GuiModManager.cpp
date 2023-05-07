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


void GuiModManager::setTriggerUpdateModsDisplayedStatus(bool triggerUpdateModsDisplayedStatus) {
  _triggerUpdateModsDisplayedStatus_ = triggerUpdateModsDisplayedStatus;
}

bool GuiModManager::isTriggerUpdateModsDisplayedStatus() const {
  return _triggerUpdateModsDisplayedStatus_;
}
const GameBrowser &GuiModManager::getGameBrowser() const { return _gameBrowser_; }
GameBrowser &GuiModManager::getGameBrowser(){ return _gameBrowser_; }

// static
void GuiModManager::applyMod(const std::string &modName_) {
  LogWarning << __METHOD_NAME__ << ": " << modName_ << std::endl;
  modApplyMonitor = ModApplyMonitor();

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

  for(size_t iFile = 0 ; iFile < modFilesList.size() ; iFile++){

    if(modFilesList[iFile][0] == '.'){
      // ignoring cached files
      continue;
    }

    std::string filePath = GenericToolbox::joinPath(modPath, modFilesList[iFile]);
    std::string fileSizeStr = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(filePath)));

    modApplyMonitor.currentFile = GenericToolbox::getFileNameFromFilePath(modFilesList[iFile]) + " (" + fileSizeStr + ")";
    modApplyMonitor.progress = double(iFile + 1) / double(modFilesList.size());

    std::string installPath = GenericToolbox::joinPath(_gameBrowser_.getModManager().fetchCurrentPreset().installBaseFolder, modFilesList[iFile] );
    GenericToolbox::Switch::IO::copyFile(filePath, installPath);
  }
  _gameBrowser_.getModManager().resetModCache(modName_);

}
void GuiModManager::getModStatus(const std::string &modName_, bool useCache_) {
  LogWarning << __METHOD_NAME__ << ": " << modName_ << ", with cache? " << useCache_ << std::endl;
  modCheckMonitor = ModCheckMonitor();

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

  modCheckMonitor.currentFile = "Listing mod files...";
  std::vector<std::string> modFileList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  for( size_t iFile = 0 ; iFile < modFileList.size() ; iFile++ ){

    std::string srcFilePath = GenericToolbox::joinPath(modPath, modFileList[iFile] );
    std::string dstFilePath = GenericToolbox::joinPath(modManager.fetchCurrentPreset().installBaseFolder, modFileList[iFile] );

    modCheckMonitor.currentFile = GenericToolbox::getFileNameFromFilePath(modFileList[iFile]);
    modCheckMonitor.progress = (double(iFile) + 1.) / double(modFileList.size());

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
  LogWarning << __METHOD_NAME__ << ": " << modName_ << std::endl;
  modRemoveMonitor = ModRemoveMonitor();

  std::string modPath = GenericToolbox::joinPath( _gameBrowser_.getModManager().getGameFolderPath(), modName_ );
  auto modFileList = GenericToolbox::getListOfFilesInSubFolders(modPath);

  int iFile{0};
  for(auto &modFile : modFileList){

    // TODO: on cancel ?

    std::string srcFilePath = GenericToolbox::joinPath( modPath, modFile );


    modRemoveMonitor.currentFile = GenericToolbox::getFileNameFromFilePath(modFile);
    modRemoveMonitor.currentFile += " (" + GenericToolbox::joinAsString("/", iFile+1, modFileList.size()) + ")";
    modRemoveMonitor.progress = (iFile++ + 1.) / double(modFileList.size());

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
void GuiModManager::removeAllMods() {
  LogWarning << __METHOD_NAME__ << std::endl;
  modRemoveAllMonitor = ModRemoveAllMonitor();

  auto modList = _gameBrowser_.getModManager().getModList();

  for(int iMod = 0 ; iMod < int(modList.size()) ; iMod++ ){
    modRemoveAllMonitor.currentMod = modList[iMod].modName + " (" + std::to_string(iMod + 1) + "/" + std::to_string(modList.size()) + ")";
    modRemoveAllMonitor.progress = (iMod + 1.) / double(modList.size());

    this->removeMod( modList[iMod].modName );
  }

}
void GuiModManager::applyModsList(std::vector<std::string>& modsList_){
  LogWarning << __METHOD_NAME__ << ": " << GenericToolbox::parseVectorAsString(modsList_) << std::endl;
  modApplyListMonitor = ModApplyListMonitor();

  // checking for overwritten files in advance:

  // All the files that will be installed are saved here
  std::vector<std::string> appliedFileList;

  // This is the filter for override files
  std::vector<std::vector<std::string>> ignoredFileListPerMod( modsList_.size() ); // ignoredFileListPerMod[iMod][iIgnoredFile];

  // looping backward as the last mods will be the ones for which their files are going to be kept
  for( int iMod = int(modsList_.size()) - 1 ; iMod >= 0 ; iMod-- ){
    std::string modPath = GenericToolbox::joinPath( _gameBrowser_.getModManager().getGameFolderPath(), modsList_[iMod] );
    auto fileList = GenericToolbox::getListOfFilesInSubFolders(modPath);
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
    // progress
    modCheckAllMonitor.currentMod = modList[iMod].modName;
    modCheckAllMonitor.currentMod += " (" + std::to_string(iMod + 1) + "/" + std::to_string(modList.size()) + ")";
    modCheckAllMonitor.progress = (double(iMod) + 1.) / double(modList.size());

    this->getModStatus( modList[iMod].modName, useCache_ );
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
  _loadingPopup_.pushView();

  LogWarning << "Applying: " << modName_ << std::endl;
  modApplyMonitor = ModApplyMonitor();
  _loadingPopup_.getMonitorView()->setHeaderTitle("Applying mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::greenNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modName_ );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modApplyMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modApplyMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &GenericToolbox::Switch::Utils::b.progressMap["copyFile"] );
  this->applyMod( modName_ );

  LogWarning << "Checking: " << modName_ << std::endl;
  modCheckMonitor = ModCheckMonitor();
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking the applied mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->getModStatus( modName_, false );

  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();
  return true;
}
bool GuiModManager::applyModPresetFunction(const std::string& presetName_){
  // push the progress bar to the view
  _loadingPopup_.pushView();

  LogInfo << "Removing all installed mods..." << std::endl;
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::redNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing all installed mods...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modRemoveAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &modRemoveMonitor.progress );
  this->removeAllMods();

  LogInfo("Applying Mod Preset");
  _loadingPopup_.getMonitorView()->setHeaderTitle("Applying mod preset...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::greenNvgColor);
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modApplyListMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modApplyMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modApplyMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["copyFile"]);

  std::vector<std::string> modsList;
  for( auto& preset : _gameBrowser_.getModPresetHandler().getPresetList() ){
    if( preset.name == presetName_ ){ modsList = preset.modList; break; }
  }
  this->applyModsList(modsList);

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking all mods status...");

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  GuiModManager::checkAllMods();

  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();
  return true;
}
bool GuiModManager::removeModFunction(const std::string& modName_){
  // push the progress bar to the view
  _loadingPopup_.pushView();

  LogWarning << "Removing: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing mod...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::redNvgColor);

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveMonitor.progress );
  GuiModManager::removeMod( modName_ );

  LogWarning << "Checking: " << modName_ << std::endl;
  _loadingPopup_.getMonitorView()->setProgressColor( GenericToolbox::Switch::Borealis::blueNvgColor );
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking mod...");

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr(&modName_);
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  GuiModManager::getModStatus(modName_);

  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();
  return true;
}
bool GuiModManager::checkAllModsFunction(){
  // push the progress bar to the view
  _loadingPopup_.pushView();

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking all mods status...");
  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr(&GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"]);
  this->checkAllMods();
  LogInfo << "Check all mods done." << std::endl;

  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();

  return true;
}
bool GuiModManager::removeAllModsFunction(){
  // push the progress bar to the view
  _loadingPopup_.pushView();

  LogInfo("Removing all installed mods...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::redNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Removing all installed mods...");

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modRemoveAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modRemoveMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modRemoveAllMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &modRemoveMonitor.progress );
  GuiModManager::removeAllMods();

  LogInfo("Checking all mods status...");
  _loadingPopup_.getMonitorView()->setProgressColor(GenericToolbox::Switch::Borealis::blueNvgColor);
  _loadingPopup_.getMonitorView()->setHeaderTitle("Checking all mods status...");

  _loadingPopup_.getMonitorView()->resetMonitorAddresses();
  _loadingPopup_.getMonitorView()->setTitlePtr( &modCheckAllMonitor.currentMod );
  _loadingPopup_.getMonitorView()->setSubTitlePtr( &modCheckMonitor.currentFile );
  _loadingPopup_.getMonitorView()->setProgressFractionPtr( &modCheckMonitor.progress );
  _loadingPopup_.getMonitorView()->setSubProgressFractionPtr( &GenericToolbox::Switch::Utils::b.progressMap["doFilesAreIdentical"] );
  GuiModManager::checkAllMods();

  _triggerUpdateModsDisplayedStatus_ = true;
  _loadingPopup_.popView();
  brls::Application::unblockInputs();
  return true;
}

