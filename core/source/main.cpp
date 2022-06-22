#include <switch.h>

#include <Toolbox.h>
#include <GlobalObjects.h>

#include "GenericToolbox.Switch.h"

// MAIN
int main(int argc, char **argv){

  consoleInit(nullptr);

  // Configure our supported input layout: a single player with standard controller styles
  padConfigureInput(1, HidNpadStyleSet_NpadStandard);

  // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
  padInitializeDefault(&GlobalObjects::gPad);

  std::string old_config_path = GenericToolbox::getCurrentWorkingDirectory() + "/parameters.ini"; // before 1.5.0
  if(GenericToolbox::doesPathIsFile(old_config_path)){
    ParametersHandler p;
    p.initialize();
    std::string new_param_file = p.get_parameters_file_path();
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft("Welcome in SimpleModManager v" + Toolbox::get_app_version(), GenericToolbox::ColorCodes::greenBackground);
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft(" > Looks like you've been running on a version <= " + Toolbox::get_app_version());
    GenericToolbox::Switch::Terminal::printLeft(" > Now parameters.ini is read from : " + new_param_file);
    GenericToolbox::Switch::Terminal::printLeft(" > The old file will be moved to this location.");
    GenericToolbox::Switch::Terminal::printLeft("");
    GenericToolbox::Switch::Terminal::printLeft("");
    Selector::ask_question("Confirm by pressing A.", {"Ok"});
    GenericToolbox::mvFile(old_config_path, new_param_file, true);
  }

  int max_depth = 1; // could be a parameter in the future

  GlobalObjects::getModBrowser().set_only_show_folders(true);
  GlobalObjects::getModBrowser().set_max_relative_depth(max_depth);
  GlobalObjects::getModBrowser().initialize();
  GlobalObjects::getModBrowser().print_menu();

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
      if(GlobalObjects::getModBrowser().get_current_relative_depth() == 0){
        break;
      }
    }

    GlobalObjects::getModBrowser().scan_inputs(kDown, kHeld);

  } // while

  consoleExit(nullptr);
  return EXIT_SUCCESS;

}