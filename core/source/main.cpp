#include <switch.h>

#include <toolbox.h>
#include <GlobalObjects.h>

// MAIN
int main(int argc, char **argv){

  consoleInit(nullptr);
  toolbox::enableEmbeddedSwitchFS();

  // Configure our supported input layout: a single player with standard controller styles
  padConfigureInput(1, HidNpadStyleSet_NpadStandard);

  // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
  padInitializeDefault(&GlobalObjects::gPad);

  std::string old_config_path = toolbox::get_working_directory() + "/parameters.ini"; // before 1.5.0
  if(toolbox::do_path_is_file(old_config_path)){
    parameters_handler p;
    p.initialize();
    std::string new_param_file = p.get_parameters_file_path();
    toolbox::print_left("");
    toolbox::print_left("Welcome in SimpleModManager v" + toolbox::get_app_version(), toolbox::green_bg);
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left(" > Looks like you've been running on a version <= " + toolbox::get_app_version());
    toolbox::print_left(" > Now parameters.ini is read from : " + new_param_file);
    toolbox::print_left(" > The old file will be moved to this location.");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::ask_question("Confirm by pressing A.", {"Ok"});
    toolbox::mv_file(old_config_path, new_param_file);
  }

  int max_depth = 1; // could be a parameter in the future

  GlobalObjects::get_mod_browser().set_only_show_folders(true);
  GlobalObjects::get_mod_browser().set_max_relative_depth(max_depth);
  GlobalObjects::get_mod_browser().initialize();
  GlobalObjects::get_mod_browser().print_menu();

  // Main loop
  u64 kDown, kHeld;
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);;

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    kDown = padGetButtonsDown(&GlobalObjects::gPad);
    kHeld = padGetButtons(&GlobalObjects::gPad);

    if(kDown & HidNpadButton_B){ // back
      if(GlobalObjects::get_mod_browser().get_current_relative_depth() == 0){
        break;
      }
    }

    GlobalObjects::get_mod_browser().scan_inputs(kDown, kHeld);

  } // while

  toolbox::disableEmbeddedSwitchFS();
  consoleExit(nullptr);
  return EXIT_SUCCESS;

}