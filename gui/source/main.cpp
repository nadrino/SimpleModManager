#include <GlobalObjects.h>
#include <toolbox.h>

#include "Logger.h"

#include <borealis.hpp>
#include <FrameRoot.h>

#include "GenericToolbox.Switch.h"

#include <string>
#include <cstdlib>

LoggerInit([]{
  Logger::setUserHeaderStr("[SimpleModManager.nro]");
});

//#include "yaml-cpp/yaml.h"

void runGui();
void runConsole();


int main(int argc, char* argv[]){
  LogInfo << "SimpleModManager is starting..." << std::endl;

  // https://github.com/jbeder/yaml-cpp/wiki/Tutorial
//  YAML::Node config = YAML::LoadFile("config.yaml");
//  if (config["lastLogin"]) {
//    std::cout << "Last logged in: " << config["lastLogin"].as<std::string>() << "\n";
//  }
//  const auto username = config["username"].as<std::string>();
//  const auto password = config["password"].as<std::string>();

  toolbox::enableEmbeddedSwitchFS();

  int max_depth = 1; // could be a parameter in the future
  GlobalObjects::getModBrowser().set_only_show_folders(true);
  GlobalObjects::getModBrowser().set_max_relative_depth(max_depth);
  GlobalObjects::getModBrowser().initialize();

  if(bool(std::stoi(GlobalObjects::getModBrowser().get_parameters_handler().get_parameter("use-gui")))){
    runGui();
  }
  else{
    runConsole();
  }

  toolbox::disableEmbeddedSwitchFS();

  // Exit
  return EXIT_SUCCESS;
}


void runGui(){
  LogThrowIf(R_FAILED(nsInitialize()), "nsInitialize Failed");

  brls::Logger::setLogLevel(brls::LogLevel::ERROR);

  LogThrowIf(not brls::Application::init("SimpleModManager"), "Unable to init Borealis application")

  auto* main_frame = new FrameRoot();
  brls::Application::pushView(main_frame);
  main_frame->registerAction("", brls::Key::PLUS, []{return true;}, true);
  main_frame->updateActionHint(brls::Key::PLUS, ""); // make the change visible

  while(brls::Application::mainLoop()) { }

  nsExit();

  if(GlobalObjects::doTriggerSwitchUI()){
    runConsole();
  }

}

void runConsole(){
  LogFatal.setMaxLogLevel();

  auto* console = consoleInit(nullptr);

  // Configure our supported input layout: a single player with standard controller styles
  padConfigureInput(1, HidNpadStyleSet_NpadStandard);

  // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
  padInitializeDefault(&GlobalObjects::gPad);

  int lastVersion = std::stoi(
      GenericToolbox::joinVectorString(
          GenericToolbox::splitString(
              GlobalObjects::getModBrowser().get_parameters_handler().get_parameter("last-program-version"),
              "."
          ),
          ""
      )
  );
  int this_version = std::stoi(
      GenericToolbox::joinVectorString(
          GenericToolbox::splitString(
              toolbox::get_app_version()
              ,"."),
          ""
      )
  );
  if(lastVersion != this_version){
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

  GlobalObjects::getModBrowser().print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    padUpdate(&GlobalObjects::gPad);

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = padGetButtonsDown(&GlobalObjects::gPad);
    u64 kHeld = padGetButtons(&GlobalObjects::gPad);

    if( (kDown & HidNpadButton_B and GlobalObjects::getModBrowser().get_current_relative_depth() == 0)
        or GlobalObjects::is_quit_now_triggered()
        ){ // back
      break;
    }

    GlobalObjects::getModBrowser().scan_inputs(kDown, kHeld);

  } // while

  consoleExit(nullptr);

}
