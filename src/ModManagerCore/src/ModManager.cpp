//
// Created by Nadrino on 06/09/2019.
//

#include "ModManager.h"
#include <Toolbox.h>
#include <ModBrowser.h>
#include "GlobalObjects.h"

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <utility>


ModManager::ModManager(ModBrowser* owner_) : _owner_(owner_) {}

// setters
void ModManager::setGameFolderPath(const std::string &gameFolderPath_) {
  _gameFolderPath_ = gameFolderPath_;
  this->updateModList();
}
void ModManager::setIgnoredFileList(std::vector<std::string>& ignoredFileList_){
  _ignoredFileList_ = ignoredFileList_;
}

// getters
const std::string & ModManager::getCurrentModFolderPath() const {
  return _gameFolderPath_;
}
const std::vector<std::string> & ModManager::getIgnoredFileList() const {
  return _ignoredFileList_;
}


void ModManager::updateModList() {

  auto folderList = GenericToolbox::getListOfSubFoldersInFolder(_gameFolderPath_);
  _modList_.clear(); _modList_.reserve( folderList.size() );
  for( auto& folder : folderList ){
    _modList_.emplace_back( folder );
  }

  std::string cacheFilePath = _gameFolderPath_ + "/mods_status_cache.txt";
  if( not GenericToolbox::doesPathIsFile(cacheFilePath) ) return;

  auto lines = GenericToolbox::dumpFileAsVectorString(cacheFilePath);
  for( auto & line : lines ){
    GenericToolbox::trimInputString(line, " ");

    if( GenericToolbox::doesStringStartsWithSubstring(line, "#") ) continue;

    auto elements = GenericToolbox::splitString(line, "=");
    for( auto& element : elements ){ GenericToolbox::trimInputString(element, " "); }

    // useless entry
    if( elements.size() < 2 ) continue;

    auto* modEntryPtr = this->fetchModEntry( elements[0] );
    if( modEntryPtr == nullptr ){ continue; }

    modEntryPtr->statusStr = elements[1];

    // v < 1.5.0
    if( elements.size() < 3 ){ continue; }

    // v >= 1.5.0
    modEntryPtr->applyFraction = std::stod( elements[2] );
  }

}
void ModManager::dumpModStatusCache() {
  std::stringstream ss;

  for( auto& mod : _modList_ ){
    ss << mod.modName << " = " << mod.statusStr << " = " << mod.applyFraction << std::endl;
  }

  std::string cacheFilePath = _gameFolderPath_ + "/mods_status_cache.txt";
  GenericToolbox::dumpStringInFile(cacheFilePath, ss.str());
}
void ModManager::resetModCache(const std::string &modName_){
  auto* modPtr = this->fetchModEntry( modName_ );
  if( modPtr == nullptr ) return;

  // reset with default field
  (*modPtr) = ModEntry(modName_);
}
void ModManager::resetAllModsCacheAndFile(){
  GenericToolbox::deleteFile(_gameFolderPath_ + "/mods_status_cache.txt");
  this->updateModList();
}

void ModManager::updateModStatus(std::string& modName_){
  auto* modPtr = this->fetchModEntry( modName_ );
  if( modPtr == nullptr ) return;

  GenericToolbox::Switch::Terminal::printLeft("   Checking : Listing mod files...", GenericToolbox::ColorCodes::magentaBackground, true);
  consoleUpdate(nullptr);

  std::string modFolderPath = _gameFolderPath_ + "/" + modPtr->modName;
  auto filesList = GenericToolbox::getListOfFilesInSubFolders( modFolderPath );

  size_t nSameFiles{0};
  size_t nIgnoredFiles{0};
  size_t iFile{0};
  for( auto& file : filesList ){
    if( _ignoreCacheFiles_ and GenericToolbox::doesStringStartsWithSubstring(GenericToolbox::getFileNameFromFilePath(file), ".") ) {
      nIgnoredFiles++; continue;
    }

    std::string installedPathCandidate{_owner_->getConfigHandler().getConfig().getCurrentPreset().installBaseFolder};
    installedPathCandidate += "/" + file;

    std::string modFilePath{modFolderPath};
    modFilePath += "/" + file;

    std::stringstream ssPbar;
    ssPbar << "Checking : (" << iFile+1 << "/" << filesList.size() << ") " << GenericToolbox::getFileNameFromFilePath(file);
    GenericToolbox::Switch::Terminal::displayProgressBar( iFile, filesList.size(), ssPbar.str() );

    if( GenericToolbox::Switch::IO::doFilesAreIdentical( installedPathCandidate, modFilePath ) ){
      nSameFiles++;
    }
  }

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE
  modPtr->applyFraction = double(nSameFiles) / double(filesList.size() - nIgnoredFiles);
  if     ( modPtr->applyFraction == 0 ){ modPtr->statusStr = "INACTIVE"; }
  else if( modPtr->applyFraction == 1 ){ modPtr->statusStr = "ACTIVE"; }
  else{
    std::stringstream ss;
    ss << "PARTIAL (" << nSameFiles << "/" << filesList.size() - nIgnoredFiles << ")";
    modPtr->statusStr = ss.str();
  }

  // immediately save
  this->dumpModStatusCache();
}

double ModManager::getModApplyFraction(const std::string &modName_){
  auto* modPtr = this->fetchModEntry( modName_ );
  if( modPtr == nullptr ) return 0;
  return modPtr->applyFraction;
}
std::string ModManager::generateStatusStr(const std::string& modName_){
  auto* modPtr = this->fetchModEntry( modName_ );
  if( modPtr == nullptr ) return {"INVALID MOD"};

  return modPtr->statusStr;
}
void ModManager::applyMod(const std::string& modName_, bool overrideConflicts_) {
  auto* modPtr = this->fetchModEntry( modName_ );
  if( modPtr == nullptr ) return;

  GenericToolbox::Switch::Terminal::printLeft("Applying : " + modName_ + "...", GenericToolbox::ColorCodes::greenBackground);
  std::string modFolderPath = _gameFolderPath_ + "/" + modPtr->modName;

  GenericToolbox::Switch::Terminal::printLeft("   Getting files list...", GenericToolbox::ColorCodes::greenBackground, true);
  auto filesList = GenericToolbox::getListOfFilesInSubFolders( modFolderPath );

  // filtering files
  GenericToolbox::removeEntryIf(filesList, [&](const std::string &file_) {
    return (
        (_ignoreCacheFiles_ and
         GenericToolbox::doesStringStartsWithSubstring(
             GenericToolbox::getFileNameFromFilePath(file_), "."
         ))
        or
        (GenericToolbox::doesElementIsInVector(file_, _ignoredFileList_))
    );
  });

  std::string onConflictAction{};
  if( overrideConflicts_ ){ onConflictAction = "Yes to all"; }

  size_t iFile{0};
  for( auto& file : filesList ){

    std::string srcFilePath{modFolderPath}; srcFilePath += "/" + file;
    std::string fileSize = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(srcFilePath)));

    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile, filesList.size(),
        "(" + std::to_string(iFile+1) + "/" + std::to_string(filesList.size()) + ") " +
        GenericToolbox::getFileNameFromFilePath(file) + " (" + fileSize + ")");
    iFile++;

    std::string dstFilePath{_owner_->getConfigHandler().getConfig().getCurrentPreset().installBaseFolder};
    dstFilePath += "/" + file;

    bool installFile{false};

    // on conflict:
    if     ( onConflictAction == "Yes to all" ){ installFile = true; } // no IO first
    else if( onConflictAction == "No to all" ){ installFile = false; }
    else if( not GenericToolbox::doesPathIsFile( dstFilePath ) ){ installFile = true; }
    else if( GenericToolbox::Switch::IO::doFilesAreIdentical( dstFilePath, srcFilePath ) ){ installFile = false; }
    else{
      onConflictAction = Selector::ask_question(
          file + " already exists. Replace ?",
          {{"Yes"}, {"Yes to all"}, {"No"}, {"No to all"}}
      );

      if( onConflictAction == "Yes" or onConflictAction == "Yes to all" ){ installFile = true; }
      if( onConflictAction == "No" or onConflictAction == "No to all" ){ installFile = false; }
    }

    if( installFile ){ GenericToolbox::Switch::IO::copyFile( srcFilePath, dstFilePath ); }

  }

  this->resetModCache( modName_ );
}
void ModManager::applyModList(const std::vector<std::string> &modNamesList_){

  // checking for overwritten files in advance
  std::vector<std::string> appliedFileList;
  std::vector<std::vector<std::string>> ignoredFileListPerMod(modNamesList_.size());

  for( int iMod = int( modNamesList_.size() ) - 1 ; iMod >= 0 ; iMod-- ){
    std::string modFolder = _gameFolderPath_ + "/" + modNamesList_[iMod];
    auto fileList = GenericToolbox::getListOfFilesInSubFolders(modFolder );
    for(auto& file : fileList){
      if( GenericToolbox::doesElementIsInVector(file, appliedFileList) ){
        ignoredFileListPerMod[iMod].emplace_back(file);
      }
      else {
        appliedFileList.emplace_back(file);
      }
    }
  }

  // applying mods with ignored files
  for( size_t iMod = 0 ; iMod < modNamesList_.size() ; iMod++ ){
    _ignoredFileList_ = ignoredFileListPerMod[iMod];
    this->applyMod( modNamesList_[iMod], true );
    _ignoredFileList_.clear();
  }

}
void ModManager::removeMod(const std::string &modName_) {

  GenericToolbox::Switch::Terminal::printLeft("Disabling : " + modName_, GenericToolbox::ColorCodes::redBackground);

  std::string srcFolder = _gameFolderPath_ + "/" + modName_;
  auto fileList{GenericToolbox::getListOfFilesInSubFolders(srcFolder)};

  size_t iFile{0};
  for( auto& file : fileList ){
    std::string srcFilePath{srcFolder};
    srcFilePath += "/" + file;

    std::string dstFilePath{_owner_->getConfigHandler().getConfig().getCurrentPreset().installBaseFolder};
    dstFilePath += "/" + file;

    std::string fileSize{
      GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(srcFilePath)))
    };

    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile++, fileList.size(),
        GenericToolbox::getFileNameFromFilePath(file) + " (" + fileSize + ")"
    );

    // Check if the installed mod belongs to the selected mod
    if( GenericToolbox::Switch::IO::doFilesAreIdentical( dstFilePath, srcFilePath ) ){

      // Remove the mod file
      GenericToolbox::deleteFile( dstFilePath );

      // Delete the folder if no other files is present
      std::string emptyFolderCandidate = GenericToolbox::getFolderPathFromFilePath(dstFilePath );
      while( GenericToolbox::isFolderEmpty( emptyFolderCandidate ) ) {
        if( emptyFolderCandidate.empty() ) break;
        GenericToolbox::deleteEmptyDirectory( emptyFolderCandidate );
        emptyFolderCandidate = GenericToolbox::getFolderPathFromFilePath( emptyFolderCandidate );
      }
    }
  }

  this->resetModCache( modName_ );
}

ModEntry* ModManager::fetchModEntry(const std::string& modName_) {
  for( auto& modEntry : _modList_ ){ if( modEntry.modName == modName_ ) return &modEntry; }
  return nullptr;
}

void ModManager::displayModFilesStatus(const std::string &modName_){

  GenericToolbox::Switch::Terminal::printLeft("Listing Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);

  std::stringstream ssSrcFolder;
  ssSrcFolder << _gameFolderPath_ << "/" << modName_;
  auto fileList = GenericToolbox::getListOfFilesInSubFolders( ssSrcFolder.str() );

  GenericToolbox::Switch::Terminal::printLeft("Checking Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);

  Selector selector(fileList);

  size_t iFile{0};
  for( auto& file : fileList ){
    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile, fileList.size(),
        "(" + std::to_string(iFile + 1) + "/" + std::to_string(fileList.size()) + ") " +
        GenericToolbox::getFileNameFromFilePath(file)
    );

    std::stringstream ssSrc;
    ssSrc << ssSrcFolder.str() << "/" << file;
    std::stringstream ssDst;
    ssDst << _owner_->getConfigHandler().getConfig().getCurrentPreset().installBaseFolder << "/" << file;

    if( GenericToolbox::Switch::IO::doFilesAreIdentical( ssDst.str(), ssSrc.str() ) ){
      selector.setTag(iFile, "-> Installed");
    }
    else if( GenericToolbox::doesPathIsFile( ssDst.str() ) ){
      selector.setTag(iFile, "-> Not same");
    }
    else{
      selector.setTag(iFile, "-> Not installed");
    }

    iFile++;
  }

  selector.setMaxItemsPerPage(GenericToolbox::Switch::Hardware::getTerminalHeight() - 9);

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while(appletMainLoop())
  {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      GenericToolbox::Switch::Terminal::printLeft(modName_, GenericToolbox::ColorCodes::redBackground);
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      selector.print();
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeft("Page (" + std::to_string(selector.getCursorPage() + 1) + "/" + std::to_string(
          selector.getNbPages()) + ")");
      std::cout << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth());
      GenericToolbox::Switch::Terminal::printLeftRight(" B : Go back", "");
      if(selector.getNbPages() > 1) GenericToolbox::Switch::Terminal::printLeftRight(" <- : Previous Page", "-> : Next Page ");
      consoleUpdate(nullptr);
    }

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);
    kHeld = padGetButtons(&GlobalObjects::gPad);

    if (kDown & HidNpadButton_B) {
      break; // break in order to return to hbmenu
    }

    selector.scanInputs(kDown, kHeld);

  }
}
