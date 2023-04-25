

#include "ConsoleHandler.h"

#include <switch.h>

#include "chrono"
#include "thread"


// test
struct FpsCap{
  explicit FpsCap(double fps_) : createDate(std::chrono::system_clock::now()), fpsMax(1/fps_) {}
  ~FpsCap(){
    auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - createDate);
    std::this_thread::sleep_for(fpsMax - timeDiff);
  }

  std::chrono::system_clock::time_point createDate;
  std::chrono::duration<double> fpsMax;
};


// MAIN
int main( int argc, char **argv ){
  consoleInit(nullptr);
  ConsoleHandler::run();
  consoleExit(nullptr);
  return EXIT_SUCCESS;
}
