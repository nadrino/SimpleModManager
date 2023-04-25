//
// Created by Adrien Blanchet on 25/04/2023.
//

#ifndef SIMPLEMODMANAGER_CONSOLEHANDLER_H
#define SIMPLEMODMANAGER_CONSOLEHANDLER_H

#include "ConfigHandler.h"
#include "Toolbox.h"
#include "Selector.h"
#include "GameBrowser.h"

#include "GenericToolbox.h"
#include "GenericToolbox.Switch.h"

#include "string"
#include "iostream"
#include "thread"
#include "chrono"


namespace ConsoleHandler{

  void upgradeFrom150(){
    std::string oldPath = GenericToolbox::getCurrentWorkingDirectory() + "/parameters.ini"; // before 1.5.0
    if(GenericToolbox::doesPathIsFile(oldPath)){
      ConfigHandler p;

      // get the new default path
      std::string newPath = p.getConfig().configFilePath;

      // load the old file
      p.loadConfig( oldPath );

      // change its path
      p.getConfig().configFilePath = newPath;

      // write to the new path
      p.dumpConfigToFile();

      // delete the old config file
      GenericToolbox::deleteFile( oldPath );

      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft("Welcome in SimpleModManager v" + Toolbox::getAppVersion(), GenericToolbox::ColorCodes::greenBackground);
      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft(" > Looks like you've been running on a version <= " + Toolbox::getAppVersion());
      GenericToolbox::Switch::Terminal::printLeft(" > Now parameters.ini is read from : " + p.getConfig().configFilePath);
      GenericToolbox::Switch::Terminal::printLeft(" > The old file has been moved to this location.");
      GenericToolbox::Switch::Terminal::printLeft("");
      GenericToolbox::Switch::Terminal::printLeft("");

      Selector::askQuestion("Confirm by pressing A.", {"Ok"});
    }
  }
  void run(){
    // legacy
    upgradeFrom150();

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    GameBrowser gameBrowser;
    gameBrowser.printTerminal();

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeAny( &pad );

    // Main loop
    u64 kDown, kHeld;
    while( appletMainLoop() ) {
//    FpsCap fpsGuard(120);

      //Scan all the inputs. This should be done once for each frame
      padUpdate( &pad );

      //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
      kDown = padGetButtonsDown( &pad );
      kHeld = padGetButtons( &pad );

      if( kDown & HidNpadButton_B ){
        // quit
        if( not gameBrowser.isGameSelected() ){
          break;
        }
      }

      gameBrowser.scanInputs( kDown, kHeld );

      if( kDown == 0 and kHeld == 0 ){
        // don't reprint
        continue;
      }

      gameBrowser.printTerminal();
      std::cout << std::flush;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } // while
  }

}

#endif //SIMPLEMODMANAGER_CONSOLEHANDLER_H
