//
// Created by Nadrino on 03/09/2019.
//

#include <GameBrowser.h>
#include <Toolbox.h>
#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>

#include <iostream>
#include <algorithm>
#include <utility>
#include <cstring>


GameBrowser::GameBrowser(){ this->init(); }

void GameBrowser::setIsGameSelected(bool isGameSelected) {
  _isGameSelected_ = isGameSelected;
}

// getters
bool GameBrowser::isGameSelected() const {
  return _isGameSelected_;
}
const ConfigHandler &GameBrowser::getConfigHandler() const {
  return _configHandler_;
}
const Selector &GameBrowser::getSelector() const{
  return _selector_;
}
Selector &GameBrowser::getSelector(){
  return _selector_;
}
ModManager &GameBrowser::getModManager(){
  return _modManager_;
}
ConfigHandler &GameBrowser::getConfigHandler(){
  return _configHandler_;
}
ModsPresetHandler &GameBrowser::getModPresetHandler(){
  return _modPresetHandler_;
}

// Browse
void GameBrowser::selectGame(const std::string &gameName_) {
  _modManager_.setGameName( gameName_ );
  _modManager_.setGameFolderPath( _configHandler_.getConfig().baseFolder + "/" + gameName_ );
  _modPresetHandler_.setModFolder( _configHandler_.getConfig().baseFolder + "/" + gameName_ );

  _isGameSelected_ = true;
}


// Terminal
void GameBrowser::scanInputs(u64 kDown, u64 kHeld){

  // nothing to do?
  if( kDown == 0 and kHeld == 0 ){ return; }

  if( _isGameSelected_ ){
    // back button pressed?
    if( kDown & HidNpadButton_B ){
      _isGameSelected_ = false;
      // will print the game browser
    }
    else{
      _modManager_.scanInputs( kDown, kHeld );
    }
    return;
  }

  // forward to the selector
  _selector_.scanInputs( kDown, kHeld );


  if     ( kDown & HidNpadButton_A ){ // select folder / apply mod
    this->selectGame( _selector_.getSelectedEntryTitle() );
  }
  else if( kDown & HidNpadButton_Y ){ // switch between config preset
    _configHandler_.selectNextPreset();
  }
  else if( kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR ){
    // switch between config preset
    auto answer = Selector::askQuestion(
        "Do you want to switch back to the GUI ?",
        std::vector<std::string>({"Yes", "No"})
    );
    if(answer == "Yes") {
      _configHandler_.getConfig().useGui = true;
      _configHandler_.dumpConfigToFile();
      consoleExit(nullptr);
      exit( EXIT_SUCCESS );
      // TODO QUIT?
//      GlobalObjects::set_quit_now_triggered(true);
    }
  }

}
void GameBrowser::printTerminal(){

  if( _isGameSelected_ ){
    _modManager_.printTerminal();
    return;
  }

  if( _selector_.getFooter().empty() ){
    // first build -> page numbering
    rebuildSelectorMenu();
  }

  // update page
  rebuildSelectorMenu();

  // print on screen
  _selector_.printTerminal();
}
void GameBrowser::rebuildSelectorMenu(){
  _selector_.clearMenu();

  _selector_.getHeader() >> "SimpleModManager v" >> Toolbox::getAppVersion() << std::endl;
  _selector_.getHeader() << GenericToolbox::ColorCodes::redBackground << "Current Folder : ";
  _selector_.getHeader() << _configHandler_.getConfig().baseFolder << std::endl;
  _selector_.getHeader() << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::endl;

  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "  Page (" << _selector_.getCursorPage() + 1 << "/" << _selector_.getNbPages() << ")" << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "Configuration preset : " << GenericToolbox::ColorCodes::greenBackground;
  _selector_.getFooter() << _configHandler_.getConfig().getCurrentPresetName() << GenericToolbox::ColorCodes::resetColor << std::endl;
  _selector_.getFooter() << "install-mods-base-folder = " + _configHandler_.getConfig().getCurrentPreset().installBaseFolder << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << " A : Select folder" >> "Y : Change config preset " << std::endl;
  _selector_.getFooter() << " B : Quit" >> "ZL/ZR : Switch back to the GUI " << std::endl;
  _selector_.getFooter() << std::endl;
  _selector_.getFooter() << std::endl;
  _selector_.getFooter() << std::endl;

  _selector_.invalidatePageCache();
  _selector_.refillPageEntryCache();
}

uint8_t* GameBrowser::getFolderIcon(const std::string& gameFolder_){
  if( _isGameSelected_ ){ return nullptr; }
  std::string game_folder_path = _configHandler_.getConfig().baseFolder + "/" + gameFolder_;
  uint8_t* icon = GenericToolbox::Switch::Utils::getFolderIconFromTitleId(GenericToolbox::Switch::Utils::lookForTidInSubFolders(game_folder_path));
  return icon;
}

// protected
void GameBrowser::init(){
  auto gameList = GenericToolbox::getListOfSubFoldersInFolder( _configHandler_.getConfig().baseFolder );

  std::vector<size_t> nGameMod;
  nGameMod.reserve( gameList.size() );
  for( auto& game : gameList ){
    nGameMod.emplace_back(
        GenericToolbox::getListOfSubFoldersInFolder(
            _configHandler_.getConfig().baseFolder + "/" + game
            ).size()
        );
  }

  auto ordering = GenericToolbox::getSortPermutation(nGameMod, [](size_t a_, size_t b_){ return a_ > b_; });
  GenericToolbox::applyPermutation(gameList, ordering);
  GenericToolbox::applyPermutation(nGameMod, ordering);

  _selector_.getEntryList().reserve( gameList.size() );
  for( size_t iGame = 0 ; iGame < gameList.size() ; iGame++ ){
    _selector_.getEntryList().emplace_back();
    _selector_.getEntryList().back().title = gameList[iGame];
    _selector_.getEntryList().back().tag = "(" + std::to_string(nGameMod[iGame]) + " mods)";
  }
}

