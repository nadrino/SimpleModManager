

#include "ConsoleHandler.h"
#include "ConfigHandler.h"
#include "Toolbox.h"

#include "Logger.h"

#include <switch.h>

#include "chrono"
#include "iostream"
#include "thread"


// MAIN
int main( int argc, char **argv ){
  Toolbox::ensureModsRootFolder();
  ConfigHandler config;

  const Result nsRc = nsInitialize();
  if( R_SUCCEEDED(nsRc) ) {
    Toolbox::ensureInstalledGameModFolders(config.getConfig().baseFolder);
  }
  else {
    LogError << "nsInitialize Failed: 0x" << std::hex << nsRc << std::dec << std::endl;
  }

  consoleInit(nullptr);
  ConsoleHandler::run();
  consoleExit(nullptr);
  if( R_SUCCEEDED(nsRc) ) {
    nsExit();
  }
  return EXIT_SUCCESS;
}
