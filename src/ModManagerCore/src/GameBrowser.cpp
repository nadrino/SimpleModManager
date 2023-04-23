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


using namespace GenericToolbox;


void GameBrowser::init(){
  auto gameList = GenericToolbox::getListOfSubFoldersInFolder( _configHandler_.getConfig().baseFolder );

  _selector_.getEntryList().reserve( gameList.size() );
  for( auto& game : gameList ){
    _selector_.getEntryList().emplace_back();
    _selector_.getEntryList().back().title = game;
  }
}

// getters
bool GameBrowser::isGameSelected() const {
  return _isGameSelected_;
}
const ConfigHandler &GameBrowser::getConfigHandler() const {
  return _configHandler_;
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
    this->selectGame( _configHandler_.getConfig().baseFolder + "/" + _selector_.getSelectedEntryTitle() );
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

  GenericToolbox::Switch::Terminal::makePause("printTerminal() done");
}
void GameBrowser::rebuildSelectorMenu(){
  _selector_.clearMenu();

  _selector_.getHeader() >> "SimpleModManager v" >> Toolbox::getAppVersion() << std::endl;
  _selector_.getHeader() << ColorCodes::redBackground << "Current Folder : ";
  _selector_.getHeader() << _configHandler_.getConfig().baseFolder << std::endl;
  _selector_.getHeader() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;

  _selector_.getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "  Page (" << _selector_.getCursorPage() + 1 << "/" << _selector_.getNbPages() << ")" << std::endl;
  _selector_.getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "Configuration preset : " << ColorCodes::greenBackground;
  _selector_.getFooter() << _configHandler_.getConfig().getCurrentPresetName() << std::endl;
  _selector_.getFooter() << "install-mods-base-folder = " + _configHandler_.getConfig().getCurrentPreset().installBaseFolder << std::endl;
  _selector_.getFooter() << repeatString("*", Switch::Hardware::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << " A : Select folder" >> "Y : Change config preset " << std::endl;
  _selector_.getFooter() << " B : Quit" >> "ZL/ZR : Switch back to the GUI " << std::endl;

  _selector_.invalidatePageCache();
  _selector_.refillPageEntryCache();
}

uint8_t* GameBrowser::getFolderIcon(const std::string& gameFolder_){
  if( _isGameSelected_ ){ return nullptr; }
  std::string game_folder_path = _configHandler_.getConfig().baseFolder + "/" + gameFolder_;
  uint8_t* icon = Switch::Utils::getFolderIconFromTitleId(Switch::Utils::lookForTidInSubFolders(game_folder_path));
  return icon;
}

