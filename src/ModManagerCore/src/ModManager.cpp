//
// Created by Nadrino on 06/09/2019.
//

#include "ModManager.h"
#include <Toolbox.h>
#include <GameBrowser.h>

#include "GenericToolbox.Switch.h"
#include "GenericToolbox.Vector.h"

#include <switch.h>

#include <iostream>
#include <sstream>


ModManager::ModManager(GameBrowser* owner_) : _owner_(owner_) {}

// setters
void ModManager::setAllowAbort(bool allowAbort) {
  _allowAbort_ = allowAbort;
}
void ModManager::setGameName(const std::string &gameName) {
  _gameName_ = gameName;
}
void ModManager::setGameFolderPath(const std::string &gameFolderPath_) {
  _gameFolderPath_ = gameFolderPath_;
  this->reloadCustomPreset();
  this->updateModList();
}
void ModManager::setIgnoredFileList(std::vector<std::string>& ignoredFileList_){
  _ignoredFileList_ = ignoredFileList_;
}

// getters
const std::string &ModManager::getGameName() const {
  return _gameName_;
}
const std::string & ModManager::getGameFolderPath() const {
  return _gameFolderPath_;
}
const Selector &ModManager::getSelector() const {
  return _selector_;
}
const std::vector<std::string> & ModManager::getIgnoredFileList() const {
  return _ignoredFileList_;
}
const std::vector<ModEntry> &ModManager::getModList() const {
  return _modList_;
}

std::vector<ModEntry> &ModManager::getModList() {
  return _modList_;
}
std::vector<std::string> & ModManager::getIgnoredFileList() {
  return _ignoredFileList_;
}

const ConfigHolder& ModManager::getConfig() const{
  return _owner_->getConfigHandler().getConfig();
}
ConfigHolder& ModManager::getConfig(){
  return _owner_->getConfigHandler().getConfig();
}

void ModManager::updateModList() {
  // list folders
  auto folderList = GenericToolbox::lsDirs(_gameFolderPath_);
  GenericToolbox::removeEntryIf(folderList, [](const std::string& entry_){ return entry_ == ".plugins"; });

  _modList_.clear(); _modList_.reserve( folderList.size() );
  for( auto& folder : folderList ){ _modList_.emplace_back( folder ); }

  // reload .txt cache
  this->reloadModStatusCache();

  // reset the selector
  _selector_ = Selector();
  for( auto& mod : _modList_ ){
    _selector_.getEntryList().emplace_back();
    _selector_.getEntryList().back().title = mod.modName;
    _selector_.getEntryList().back().tag = mod.applyCache[this->fetchCurrentPreset().name].statusStr;
  }

  _selector_.clearMenu();
}
void ModManager::dumpModStatusCache() {
  std::stringstream ss;

  for( auto& mod : _modList_ ){
    for( auto& presetCache : mod.applyCache ){
      ss << presetCache.first << ": " << mod.modName
      << " = " << presetCache.second.statusStr
      << " = " << presetCache.second.applyFraction << std::endl;
    }
  }

  std::string cacheFilePath = _gameFolderPath_ + "/mods_status_cache.txt";
  GenericToolbox::dumpStringInFile(cacheFilePath, ss.str());
}
void ModManager::reloadModStatusCache(){
  std::string cacheFilePath = _gameFolderPath_ + "/mods_status_cache.txt";
  if( not GenericToolbox::isFile(cacheFilePath) ) return;

  auto lines = GenericToolbox::dumpFileAsVectorString( cacheFilePath );
  for( auto & line : lines ){
    GenericToolbox::trimInputString(line, " ");

    if( GenericToolbox::startsWith(line, "#") ) continue;

    auto elements = GenericToolbox::splitString(line, "=");
    if( elements.size() < 2 ) continue;
    for( auto& element : elements ){ GenericToolbox::trimInputString(element, " "); }

    // split config preset "default: Mod name ..."
    auto presetModName = GenericToolbox::splitString(elements[0], ":");
    if( presetModName.size() != 2 ){ continue; }
    for( auto& element : presetModName ){ GenericToolbox::trimInputString(element, " "); }

    int modIndex = this->getModIndex( presetModName[1] );
    if( modIndex == -1 ) continue;
    auto* modEntryPtr = &_modList_[modIndex];

    modEntryPtr->applyCache[presetModName[0]].statusStr = elements[1];

    // v < 1.5.0
    if( elements.size() < 3 ){ continue; }

    // v >= 1.5.0
    modEntryPtr->applyCache[presetModName[0]].applyFraction = std::stod( elements[2] );
  }
}
void ModManager::resetAllModsCacheAndFile(){
  GenericToolbox::rm(_gameFolderPath_ + "/mods_status_cache.txt");
  this->updateModList();
}

// mod management
void ModManager::resetModCache(int modIndex_){
  if( modIndex_ < 0 or modIndex_ >= int( _modList_.size() ) ) return;
  auto* modPtr = &_modList_[modIndex_];
  if( modPtr == nullptr ) return;

  // reset with default field
  (*modPtr) = ModEntry(modPtr->modName);
}
void ModManager::resetModCache(const std::string &modName_){
  this->resetModCache( this->getModIndex(modName_) );
}

ResultModAction ModManager::updateModStatus(int modIndex_){
  if( modIndex_ < 0 or modIndex_ >= int( _modList_.size() ) ) return Fail;
  auto* modPtr = &_modList_[modIndex_];
  if( modPtr == nullptr ) return Fail;

  GenericToolbox::Switch::Terminal::printLeft("Checking : " + modPtr->modName, GenericToolbox::ColorCodes::magentaBackground);
  consoleUpdate(nullptr);

  std::string modFolderPath = _gameFolderPath_ + "/" + modPtr->modName;
  auto filesList = GenericToolbox::lsFilesRecursive( modFolderPath );


  PadState pad;
  padInitializeAny(&pad);

  size_t nSameFiles{0};
  size_t nIgnoredFiles{0};
  size_t iFile{0};
  for( auto& file : filesList ){

    if( _allowAbort_ ){
      padUpdate( &pad );
      auto kDown = padGetButtonsDown( &pad );
      if( kDown & HidNpadButton_B ) return Abort;
    }

    if( _ignoreCacheFiles_ and GenericToolbox::startsWith(GenericToolbox::getFileName(file), ".") ) {
      nIgnoredFiles++; continue;
    }

    std::string installedPathCandidate{this->fetchCurrentPreset().installBaseFolder};
    installedPathCandidate += "/" + file;

    std::string modFilePath{modFolderPath};
    modFilePath += "/" + file;

    std::stringstream ssPbar;
    ssPbar << "Checking : (" << iFile+1 << "/" << filesList.size() << ") " << GenericToolbox::getFileName(file);
    GenericToolbox::Switch::Terminal::displayProgressBar( iFile++, filesList.size(), ssPbar.str() );

    if( GenericToolbox::Switch::IO::doFilesAreIdentical( installedPathCandidate, modFilePath ) ){
      nSameFiles++;
    }
  }

  // (XX/XX) Files Applied
  // ACTIVE
  // INACTIVE
  modPtr->applyCache[this->fetchCurrentPreset().name].applyFraction = double(nSameFiles) / double(filesList.size() - nIgnoredFiles);
  if     ( filesList.empty() )         { modPtr->applyCache[this->fetchCurrentPreset().name].statusStr = "NO FILE"; }
  else if( modPtr->applyCache[this->fetchCurrentPreset().name].applyFraction == 0 ){ modPtr->applyCache[this->fetchCurrentPreset().name].statusStr = "INACTIVE"; }
  else if( modPtr->applyCache[this->fetchCurrentPreset().name].applyFraction == 1 ){ modPtr->applyCache[this->fetchCurrentPreset().name].statusStr = "ACTIVE"; }
  else{
    std::stringstream ss;
    ss << "PARTIAL (" << nSameFiles << "/" << filesList.size() - nIgnoredFiles << ")";
    modPtr->applyCache[this->fetchCurrentPreset().name].statusStr = ss.str();
  }

  // update selector
  _selector_.setTag(modIndex_, modPtr->applyCache[this->fetchCurrentPreset().name].statusStr);

  // immediately save
  this->dumpModStatusCache();

  return Success;
}
ResultModAction ModManager::updateModStatus(const std::string& modName_){
  return this->updateModStatus( this->getModIndex(modName_) );
}
ResultModAction ModManager::updateAllModStatus(){
  _selector_.clearTags();

  PadState pad;
  padInitializeAny(&pad);

  for(size_t iMod = 0 ; iMod < _selector_.getEntryList().size() ; iMod++ ){

    if( _allowAbort_ ){
      padUpdate( &pad );
      u64 kDown = padGetButtonsDown(&pad);
      if(kDown & HidNpadButton_B) return Abort;
    }

    _selector_.setTag(iMod, "Checking...");
    printTerminal();
    std::stringstream ss;
    ss << "Checking (" << iMod + 1 << "/" << _selector_.getEntryList().size() << ") : ";
    ss << _selector_.getEntryList()[iMod].title << "...";
    GenericToolbox::Switch::Terminal::printLeft(ss.str(), GenericToolbox::ColorCodes::magentaBackground);
    consoleUpdate(nullptr);
    auto result = this->updateModStatus( _selector_.getEntryList()[iMod].title );
    if( result == Abort ) return result;
  }

  return Success;
}

ResultModAction ModManager::applyMod(int modIndex_, bool overrideConflicts_){
  if( modIndex_ < 0 or modIndex_ >= int( _modList_.size() ) ) return Fail;
  auto* modPtr = &_modList_[modIndex_];
  if( modPtr == nullptr ) return Fail;

  GenericToolbox::Switch::Terminal::printLeft("Applying : " + modPtr->modName + "...", GenericToolbox::ColorCodes::greenBackground);
  consoleUpdate(nullptr);
  std::string modFolderPath = _gameFolderPath_ + "/" + modPtr->modName;

  GenericToolbox::Switch::Terminal::printLeft("   Getting files list...", GenericToolbox::ColorCodes::greenBackground, true);
  consoleUpdate(nullptr);
  auto filesList = GenericToolbox::lsFilesRecursive( modFolderPath );

  // filtering files
  GenericToolbox::removeEntryIf(filesList, [&](const std::string &file_) {
    return (
        (_ignoreCacheFiles_ and
        GenericToolbox::startsWith(
            GenericToolbox::getFileName(file_), "."
         ))
        or
        (GenericToolbox::doesElementIsInVector(file_, _ignoredFileList_))
    );
  });

  std::string onConflictAction{};
  if( overrideConflicts_ ){ onConflictAction = "Yes to all"; }

  PadState pad;
  padInitializeAny(&pad);

  this->resetModCache( modPtr->modName );

  size_t iFile{0};
  for( auto& file : filesList ){

    if( _allowAbort_ ){
      padUpdate( &pad );
      auto kDown = padGetButtonsDown(&pad);
      if( kDown & HidNpadButton_B ){ return Abort; }
    }

    std::string srcFilePath{modFolderPath}; srcFilePath += "/" + file;
    std::string fileSize = GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(srcFilePath)));

    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile, filesList.size(),
        "(" + std::to_string(iFile+1) + "/" + std::to_string(filesList.size()) + ") " +
        GenericToolbox::getFileName(file) + " (" + fileSize + ")"
    );
    iFile++;

    std::string dstFilePath{this->fetchCurrentPreset().installBaseFolder};
    dstFilePath += "/" + file;

    bool installFile{false};

    // on conflict:
    if     ( onConflictAction == "Yes to all" ){ installFile = true; } // no IO first
    else if( onConflictAction == "No to all" ){ installFile = false; }
    else if( not GenericToolbox::isFile( dstFilePath ) ){ installFile = true; }
    else if( GenericToolbox::Switch::IO::doFilesAreIdentical( dstFilePath, srcFilePath ) ){ installFile = false; }
    else{
      onConflictAction = Selector::askQuestion(
          file + " already exists. Replace ?",
          {{"Yes"},
           {"Yes to all"},
           {"No"},
           {"No to all"}}
      );

      if( onConflictAction == "Yes" or onConflictAction == "Yes to all" ){ installFile = true; }
      if( onConflictAction == "No" or onConflictAction == "No to all" ){ installFile = false; }
    }

    if( installFile ){ GenericToolbox::Switch::IO::copyFile( srcFilePath, dstFilePath ); }

  }

  return Success;
}
ResultModAction ModManager::applyMod(const std::string& modName_, bool overrideConflicts_) {
  return this->applyMod( this->getModIndex(modName_), overrideConflicts_ );
}
ResultModAction ModManager::applyModList(const std::vector<std::string> &modNamesList_){
  // checking for overwritten files in advance
  std::vector<std::string> appliedFileList;
  std::vector<std::vector<std::string>> ignoredFileListPerMod(modNamesList_.size());

  for( int iMod = int( modNamesList_.size() ) - 1 ; iMod >= 0 ; iMod-- ){
    std::string modFolder = _gameFolderPath_ + "/" + modNamesList_[iMod];
    auto fileList = GenericToolbox::lsFilesRecursive(modFolder );
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

    auto result = this->applyMod( modNamesList_[iMod], true );
    if( result == Abort ){ return result; }

    _ignoredFileList_.clear();
  }

  return Success;
}

void ModManager::removeMod(int modIndex_) {
  if( modIndex_ < 0 or modIndex_ >= int( _modList_.size() ) ) return;
  auto* modPtr = &_modList_[modIndex_];

  GenericToolbox::Switch::Terminal::printLeft("Disabling : " + modPtr->modName, GenericToolbox::ColorCodes::redBackground);

  std::string srcFolder = _gameFolderPath_ + "/" + modPtr->modName;
  auto fileList{GenericToolbox::lsFilesRecursive(srcFolder)};

  size_t iFile{0};
  for( auto& file : fileList ){
    std::string srcFilePath{srcFolder};
    srcFilePath += "/" + file;

    std::string dstFilePath{this->fetchCurrentPreset().installBaseFolder};
    dstFilePath += "/" + file;

    std::string fileSize{
        GenericToolbox::parseSizeUnits(double(GenericToolbox::getFileSize(srcFilePath)))
    };

    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile++, fileList.size(),
        GenericToolbox::getFileName(file) + " (" + fileSize + ")"
    );

    // Check if the installed mod belongs to the selected mod
    if( GenericToolbox::Switch::IO::doFilesAreIdentical( dstFilePath, srcFilePath ) ){

      // Remove the mod file
      GenericToolbox::rm( dstFilePath );

      // Delete the folder if no other files is present
      std::string emptyFolderCandidate = GenericToolbox::getFolderPath(dstFilePath );
      while( GenericToolbox::isDirEmpty( emptyFolderCandidate ) ) {
        if( emptyFolderCandidate.empty() ) break;
        GenericToolbox::rmDir( emptyFolderCandidate );
        emptyFolderCandidate = GenericToolbox::getFolderPath( emptyFolderCandidate );
      }
    }
  }

  this->resetModCache( modPtr->modName );
}
void ModManager::removeMod(const std::string &modName_) {
  this->removeMod( this->getModIndex(modName_) );
}


// terminal
void ModManager::scanInputs(u64 kDown, u64 kHeld){

  _selector_.scanInputs( kDown, kHeld );

  if     ( kDown & HidNpadButton_A ){
    // Apply mod
    this->applyMod( int( _selector_.getCursorPosition() )  );
    this->updateModStatus( int( _selector_.getCursorPosition() ) );
  }
  else if( kDown & HidNpadButton_X ){
    // disable mod
    this->removeMod( int( _selector_.getCursorPosition() ) );
    this->updateModStatus( int( _selector_.getCursorPosition() ) );
  }
  else if( kDown & HidNpadButton_Y ){ // mod detailed infos
    std::vector<std::string> answers{
        {"Show the status of each mod files"},
        {"Show the list of conflicts"}
    };
    auto answer = Selector::askQuestion( "Mod options:", answers );
    if     ( answer == answers[0] ){
      this->displayModFilesStatus( _selector_.getSelectedEntryTitle() );
    }
    else if( answer == answers[1] ){
      this->displayConflictsWithOtherMods( _selector_.getCursorPosition() );
    }
  }
  else if( kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR ){ // recheck all mods
    std::vector<std::string> answers{
        {"Reset mods status cache and recheck all mods"},
        {"Disable all installed mods"},
        {"Attribute a config preset for this folder"}
    };
    auto answer = Selector::askQuestion( "Options for this folder:", answers );

    if     ( answer == answers[0] ){
      auto subAnswer = Selector::askQuestion("Do you which to recheck all mods ?", {{"Yes"}, {"No"}});
      if( subAnswer == "Yes"){
        this->resetAllModsCacheAndFile();
        for( auto& mod : _modList_ ){ this->updateModStatus( mod.modName ); }
      }
    }
    else if( answer == answers[1] ){
      auto subAnswer = Selector::askQuestion("Confirm to delete all mods", {{"Yes"}, {"No"}});
      if( subAnswer == "Yes"){
        for( auto& mod : _modList_ ){ this->removeMod( mod.modName ); }
        for( auto& mod : _modList_ ){ this->updateModStatus( mod.modName ); }
      }
    }
    else if( answer == answers[2] ){

      std::vector<std::string> presetsList;
      std::vector<std::vector<std::string>> descriptionLines;

      presetsList.emplace_back("Keep the main menu preset (default)." );
      descriptionLines.emplace_back();

      for( auto& preset: _owner_->getConfigHandler().getConfig().presetList ){
        presetsList.emplace_back(preset.name );
        descriptionLines.emplace_back();
        descriptionLines.back().emplace_back(
            "install-mods-base-folder: " + preset.name +"-install-mods-base-folder"
        );
      }

      auto selectedPreset = Selector::askQuestion(
          "Please select the config preset you want for this folder:",
          presetsList, descriptionLines
      );

      // overwriting
      if( selectedPreset != presetsList[0] ){
        this->setCustomPreset( selectedPreset );
      }
      else{
        this->setCustomPreset( "" );
      }
    }
  }
  else if( kDown & HidNpadButton_Minus ){
    // Enter the mods preset menu a mods preset
    _owner_->getModPresetHandler().selectModPreset();
  }
  else if( kDown & HidNpadButton_Plus ){
    // Apply a mods preset
    std::string answer = Selector::askQuestion(
        "Do you want to apply " + _owner_->getModPresetHandler().getSelectedModPresetName() + " ?",
        {{"Yes"}, {"No"}}
    );
    if( answer == "Yes" ){
      this->applyModList( _owner_->getModPresetHandler().getSelectedPresetModList() );
      this->updateAllModStatus();
    }
  }
  else if( kDown & HidNpadButton_L ){
    _owner_->getModPresetHandler().getSelector().selectPrevious();
  }
  else if( kDown & HidNpadButton_R ){
    _owner_->getModPresetHandler().getSelector().selectNextEntry();
  }

}
void ModManager::printTerminal(){

  // first build?
  if( _selector_.getFooter().empty() ){
    // -> page numbering
    rebuildSelectorMenu();
  }

  // update page
  rebuildSelectorMenu();

  // print on screen
  _selector_.printTerminal();
}
void ModManager::rebuildSelectorMenu(){
  _selector_.clearMenu();
  _selector_.getHeader() >> "SimpleModManager v" >> Toolbox::getAppVersion() << std::endl;
  _selector_.getHeader() << GenericToolbox::ColorCodes::redBackground << "Current Folder : " << _owner_->getConfigHandler().getConfig().baseFolder << std::endl;
  _selector_.getHeader() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;

  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "  Page (" << _selector_.getCursorPage() + 1 << "/" << _selector_.getNbPages() << ")" << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "Mod preset : " << _owner_->getModPresetHandler().getSelectedModPresetName() << std::endl;
  _selector_.getFooter() << "Configuration preset : " << GenericToolbox::ColorCodes::greenBackground;
  _selector_.getFooter() << fetchCurrentPreset().name << GenericToolbox::ColorCodes::resetColor << std::endl;
  _selector_.getFooter() << "install-mods-base-folder = " + this->fetchCurrentPreset().installBaseFolder << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << " ZL : Rescan all mods" >> "ZR : Disable all mods " << std::endl;
  _selector_.getFooter() << " A/X : Apply/Disable mod" >> "L/R : Previous/Next preset " << std::endl;
  _selector_.getFooter() << " -/+ : Select/Apply mod preset" >> "Y : Mod options " << std::endl;
  _selector_.getFooter() << " B : Go back" << std::endl;
  _selector_.getFooter() << std::endl;
  _selector_.getFooter() << std::endl;

  _selector_.invalidatePageCache();
  _selector_.refillPageEntryCache();
}
void ModManager::displayModFilesStatus(const std::string &modName_){

  GenericToolbox::Switch::Terminal::printLeft("Listing Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);

  std::stringstream ssSrcFolder;
  ssSrcFolder << _gameFolderPath_ << "/" << modName_;
  auto fileList = GenericToolbox::lsFilesRecursive( ssSrcFolder.str() );

  GenericToolbox::Switch::Terminal::printLeft("Checking Files...", GenericToolbox::ColorCodes::redBackground);
  consoleUpdate(nullptr);

  Selector selector(fileList);

  size_t iFile{0};
  for( auto& file : fileList ){
    GenericToolbox::Switch::Terminal::displayProgressBar(
        iFile, fileList.size(),
        "(" + std::to_string(iFile + 1) + "/" + std::to_string(fileList.size()) + ") " +
        GenericToolbox::getFileName(file)
    );

    std::stringstream ssSrc;
    ssSrc << ssSrcFolder.str() << "/" << file;
    std::stringstream ssDst;
    ssDst << this->fetchCurrentPreset().installBaseFolder << "/" << file;

    if( GenericToolbox::Switch::IO::doFilesAreIdentical( ssDst.str(), ssSrc.str() ) ){
      selector.setTag(iFile, "-> Installed");
    }
    else if( GenericToolbox::isFile( ssDst.str() ) ){
      selector.setTag(iFile, "-> Not same");
    }
    else{
      selector.setTag(iFile, "-> Not installed");
    }

    iFile++;
  }

  selector.getHeader() << GenericToolbox::ColorCodes::redBackground << modName_ << std::endl;
  selector.getHeader() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth());

  selector.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  selector.getFooter() << "Page (" << selector.getCursorPage() << "/" << selector.getNbPages() << ")" << std::endl;
  selector.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  selector.getFooter() << " B : Go back" << std::endl;
  if(selector.getNbPages() > 1) selector.getFooter() << " <- : Previous Page" >> "-> : Next Page " << std::endl;

  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while( appletMainLoop() ){

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      selector.printTerminal();
      consoleUpdate(nullptr);
    }

    PadState pad;
    padInitializeAny(&pad);

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&pad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&pad);
    kHeld = padGetButtons(&pad);

    if (kDown & HidNpadButton_B) {
      break; // break in order to return to hbmenu
    }

    selector.scanInputs(kDown, kHeld);

  }
}
void ModManager::displayConflictsWithOtherMods(size_t modIndex_){

  consoleClear();

  struct ModFileConflict{
    std::string file{};
    std::vector<std::string> conflictingModList{};
  };


  auto fileList = GenericToolbox::lsFilesRecursive(
      _gameFolderPath_ + "/" + _modList_[modIndex_].modName
  );
  std::vector<ModFileConflict> modFileConflictList;

  // create entries for files of the given mod
  modFileConflictList.reserve( fileList.size() );
  for( auto& file : fileList ){
    modFileConflictList.emplace_back();
    modFileConflictList.back().file = file;
  }

  // now loop over all the other mods to find collisions
  for( auto& mod : _modList_ ){
    if( mod.modName == _modList_[modIndex_].modName ){ continue; }

    auto fList = GenericToolbox::lsFilesRecursive( _gameFolderPath_+"/"+mod.modName );
    for( auto& conflictCandidate : modFileConflictList ){
      if( GenericToolbox::doesElementIsInVector(conflictCandidate.file, fList) ){
        conflictCandidate.conflictingModList.emplace_back( mod.modName );
      }
    }
  }

  // building the selector
  Selector sel;

  for( auto& fileConflict : modFileConflictList ){
    if( fileConflict.conflictingModList.empty() ) continue;
    sel.getEntryList().emplace_back();
    sel.getEntryList().back().title = fileConflict.file;
    sel.getEntryList().back().description = fileConflict.conflictingModList;
  }

  if( sel.getEntryList().empty() ){
    sel.getEntryList().emplace_back();
    sel.getEntryList().back().title = "This mod has no conflicts.";
  }

  auto rebuildHeader = [&]{
    sel.clearMenu();

    // header
    sel.getHeader() << GenericToolbox::ColorCodes::redBackground << "Conflicts with " + _modList_[modIndex_].modName << std::endl;
    sel.getHeader() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;

    // footer
    sel.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
    sel.getFooter() << "Page (" << sel.getCursorPage() << "/" << sel.getNbPages() << ")" << std::endl;
    sel.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
    sel.getFooter() << " B : Go back" >> "" << std::endl;
    sel.getFooter() << " <- : Previous Page" >> "-> : Next Page " << std::endl;

    sel.invalidatePageCache();
    sel.refillPageEntryCache();
  };

  // first page computation
  rebuildHeader();


  // Main loop
  u64 kDown = 1;
  u64 kHeld = 1;
  while( appletMainLoop() ) {

    if(kDown != 0 or kHeld != 0){
      consoleClear();
      rebuildHeader(); // update page
      _selector_.printTerminal();
      consoleUpdate(nullptr);
    }

    PadState pad;
    padInitializeAny(&pad);

    //Scan all the inputs. This should be done once for each frame
    padUpdate(&pad);

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&pad);
    kHeld = padGetButtons(&pad);

    if (kDown & HidNpadButton_B) {
      break; // break in order to return to hbmenu
    }

    sel.scanInputs(kDown, kHeld);
  }

}

// utils
int ModManager::getModIndex(const std::string& modName_){
  return GenericToolbox::findElementIndex(modName_, _modList_, [](const ModEntry& mod_){ return mod_.modName; } );
}


void ModManager::reloadCustomPreset(){
  std::string configFilePath = _gameFolderPath_ + "/this_folder_config.txt";
  if( GenericToolbox::isFile(configFilePath) ){
    _currentPresetName_ = GenericToolbox::dumpFileAsString(configFilePath);
  }
  else{
    _currentPresetName_ = "";
  }
}
void ModManager::setCustomPreset(const std::string &presetName_) {
  std::string configFilePath = _gameFolderPath_ + "/this_folder_config.txt";
  GenericToolbox::rm( configFilePath );
  if( not presetName_.empty() ){
    GenericToolbox::dumpStringInFile( configFilePath, presetName_ );
  }
  this->reloadCustomPreset();
}

const PresetConfig &ModManager::fetchCurrentPreset() const {
  int idx = GenericToolbox::findElementIndex(_currentPresetName_, getConfig().presetList, [](const PresetConfig& p){ return p.name; });
  if( idx != -1 ){ return getConfig().presetList[idx]; }
  return getConfig().getCurrentPreset();
}

const std::string &ModManager::getCurrentPresetName() const {
  return _currentPresetName_;
}






