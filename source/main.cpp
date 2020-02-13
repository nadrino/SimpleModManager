#include <switch.h>

#include <mod_browser.h>

mod_browser __mod_browser__;

// MAIN
int main(int argc, char **argv){

  consoleInit(nullptr);

  int max_depth = 1; // could be a parameter in the future

  __mod_browser__.set_only_show_folders(true);
  __mod_browser__.set_max_depth(max_depth);
  __mod_browser__.initialize();
  __mod_browser__.print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if(kDown & KEY_B and __mod_browser__.get_current_depth() == 0){ // back
      break;
    }

    __mod_browser__.scan_inputs(kDown, kHeld);

  } // while

  consoleExit(nullptr);
  return 0;
}