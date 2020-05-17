#include <switch.h>

#include <mod_browser.h>
#include <toolbox.h> // keep it for debug

//#include <SDL2/SDL.h>
#include <application_handler.h>


// MAIN
int main(int argc, char **argv){



  application_handler app;
  app.initialize();
  app.start_main_loop();
  app.reset();

  return EXIT_SUCCESS;

}