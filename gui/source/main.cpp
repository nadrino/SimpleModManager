#include <stdio.h>
#include <stdlib.h>

#include <toolbox.h>

#include <borealis.hpp>
#include <string>
#include <GlobalObjects.h>
#include <main_application.h>

bool __is_new_version__;
void run_gui();
void run_console();


int main(int argc, char* argv[])
{

  toolbox::enableEmbeddedSwitchFS();

  int max_depth = 1; // could be a parameter in the future
  GlobalObjects::get_mod_browser().set_only_show_folders(true);
  GlobalObjects::get_mod_browser().set_max_relative_depth(max_depth);
  GlobalObjects::get_mod_browser().initialize();

  __is_new_version__ = false;
  int last_version = std::stoi(
    toolbox::join_vector_string(
      toolbox::split_string(
        GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("last-program-version"),
        "."
      ),
      ""
    )
  );
  int this_version = std::stoi(
    toolbox::join_vector_string(
      toolbox::split_string(
        toolbox::get_app_version()
        ,"."),
      ""
    )
  );
  if(last_version != this_version){
    __is_new_version__ = true;
  }

  if(bool(std::stoi(GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("use-gui")))){
    run_gui();
  }
  else{
    run_console();
  }

  toolbox::disableEmbeddedSwitchFS();

  // Exit
  return EXIT_SUCCESS;
}


void run_gui(){

  Result rc;
  rc = nsInitialize();
  if (R_FAILED(rc)){
    brls::Logger::debug("nsInitialize Failed");
  }

  main_application app;
  app.initialize();
  app.start_loop();

  nsExit();

}

void run_console(){

  if(__is_new_version__){
    toolbox::print_left("");
    toolbox::print_left("Welcome in SimpleModManager v" + toolbox::get_app_version(), toolbox::green_bg);
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left(" > The application have successfully been upgraded.");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::ask_question("To continue, press A.", {"Ok"});
  }

  GlobalObjects::get_mod_browser().print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if(kDown & KEY_B and GlobalObjects::get_mod_browser().get_current_relative_depth() == 0){ // back
      break;
    }

    GlobalObjects::get_mod_browser().scan_inputs(kDown, kHeld);

  } // while

}
