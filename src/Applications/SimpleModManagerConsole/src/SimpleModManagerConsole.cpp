

#include "GameBrowser.h"
#include <GlobalObjects.h>
#include <Toolbox.h>

#include "GenericToolbox.Switch.h"

#include <switch.h>


using namespace GenericToolbox::Switch;


void upgradeFrom150();


// MAIN
int main( int argc, char **argv ){

  consoleInit(nullptr);

  // Configure our supported input layout: a single player with standard controller styles
  padConfigureInput(1, HidNpadStyleSet_NpadStandard);

  GameBrowser gameBrowser;
  gameBrowser.init();
  gameBrowser.printTerminal();

  GenericToolbox::Switch::Terminal::makePause("init pad...");

  // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
  PadState mainPad;
  padInitializeAny( &mainPad );

  // Main loop
  u64 kDown, kHeld;
  while( appletMainLoop() ) {
    GenericToolbox::Switch::Terminal::makePause("starting loop");

    //Scan all the inputs. This should be done once for each frame
    padUpdate( &mainPad );

    GenericToolbox::Switch::Terminal::makePause("pad updated");

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown( &mainPad );
    kHeld = padGetButtons( &mainPad );

    GenericToolbox::Switch::Terminal::makePause(GET_VAR_NAME_VALUE(kDown));

    if( kDown == 0 and kHeld == 0 ){
      // nothing to reprocess
      continue;
    }

    if( kDown & HidNpadButton_B ){
      // quit
      if( not gameBrowser.isGameSelected() ){
        break;
      }
    }

    GenericToolbox::Switch::Terminal::makePause( "re-scan inputs gBrowser" );
    gameBrowser.scanInputs( kDown, kHeld );
    GenericToolbox::Switch::Terminal::makePause( "print term?" );
    gameBrowser.printTerminal();

  } // while

  consoleExit(nullptr);
  return EXIT_SUCCESS;
}

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

    Terminal::printLeft("");
    Terminal::printLeft("Welcome in SimpleModManager v" + Toolbox::getAppVersion(), GenericToolbox::ColorCodes::greenBackground);
    Terminal::printLeft("");
    Terminal::printLeft("");
    Terminal::printLeft("");
    Terminal::printLeft("");
    Terminal::printLeft(" > Looks like you've been running on a version <= " + Toolbox::getAppVersion());
    Terminal::printLeft(" > Now parameters.ini is read from : " + p.getConfig().configFilePath);
    Terminal::printLeft(" > The old file has been moved to this location.");
    Terminal::printLeft("");
    Terminal::printLeft("");

    Selector::askQuestion("Confirm by pressing A.", {"Ok"});
  }

}