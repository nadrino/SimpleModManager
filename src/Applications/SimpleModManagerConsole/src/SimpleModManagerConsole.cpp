

#include "ConsoleHandler.h"

#include <switch.h>

#include "chrono"
#include "thread"


// MAIN
int main( int argc, char **argv ){
  consoleInit(nullptr);
  ConsoleHandler::run();
  consoleExit(nullptr);
  return EXIT_SUCCESS;
}
