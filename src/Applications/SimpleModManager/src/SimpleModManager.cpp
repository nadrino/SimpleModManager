//
// Created by Adrien Blanchet on 14/04/2023.
//


#include "SimpleModManager.h"

#include <FrameRoot.h>
#include <GlobalObjects.h>
#include <Toolbox.h>

#include "ConfigHandler.h"

#include "Logger.h"
#include "GenericToolbox.Switch.h"

#include <borealis.hpp>

#include <string>
#include <cstdlib>
#include "iostream"

#include "switch.h"


LoggerInit([]{
  Logger::setUserHeaderStr("[SimpleModManager.nro]");
});


int main(int argc, char* argv[]){
  LogInfo << "SimpleModManager is starting..." << std::endl;

  // https://github.com/jbeder/yaml-cpp/wiki/Tutorial
//  YAML::Node config = YAML::LoadFile("config.yaml");
//  if (config["lastLogin"]) {
//    std::cout << "Last logged in: " << config["lastLogin"].as<std::string>() << "\n";
//  }
//  const auto username = config["username"].as<std::string>();
//  const auto password = config["password"].as<std::string>();

//  ConfigHandler c;

  runGui();

//  if( c.getConfig().useGui ){ runGui(); }
//  else                      { runConsole(); }

  // Exit
  return EXIT_SUCCESS;
}


void runGui(){
  LogInfo << "Starting GUI..." << std::endl;
  LogThrowIf(R_FAILED(nsInitialize()), "nsInitialize Failed");

  brls::Logger::setLogLevel(brls::LogLevel::ERROR);

  brls::i18n::loadTranslations("en-US");
  LogThrowIf(not brls::Application::init("SimpleModManager"), "Unable to init Borealis application");

  auto* main_frame = new FrameRoot();
  brls::Application::pushView(main_frame);
  main_frame->registerAction("", brls::Key::PLUS, []{return true;}, true);
  main_frame->updateActionHint(brls::Key::PLUS, ""); // make the change visible

  while(brls::Application::mainLoop()) { }

  nsExit();
}

void runConsole(){
  LogInfo << "Starting Console..." << std::endl;
  LogFatal.setMaxLogLevel(); // mute every instance of the logger

  auto* console = consoleInit(nullptr);
//
//  // Configure our supported input layout: a single player with standard controller styles
//  padConfigureInput(1, HidNpadStyleSet_NpadStandard);
//
//  PadState pad;
//
//  // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
//  padInitializeAny(&pad);
//
//  int lastVersion = std::stoi(
//      GenericToolbox::joinVectorString(
//          GenericToolbox::splitString(
//              GlobalObjects::gGameBrowser.getConfigHandler().get_parameter("last-program-version"),
//              "."
//          ),
//          ""
//      )
//  );
//  int this_version = std::stoi(
//      GenericToolbox::joinVectorString(
//          GenericToolbox::splitString(
//              Toolbox::getAppVersion()
//              , "."),
//          ""
//      )
//  );
//  if(lastVersion != this_version){
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft("Welcome in SimpleModManager v" + Toolbox::getAppVersion(), GenericToolbox::ColorCodes::greenBackground);
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft(" > The application have successfully been upgraded.");
//    GenericToolbox::Switch::Terminal::printLeft("");
//    GenericToolbox::Switch::Terminal::printLeft("");
//    Selector::askQuestion("To continue, press A.", {"Ok"});
//  }
//
//  GlobalObjects::gGameBrowser.printTerminal();
//
//  // Main loop
//  while(appletMainLoop())
//  {
//    //Scan all the inputs. This should be done once for each frame
//    padUpdate(&pad);
//
//    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
//    u64 kDown = padGetButtonsDown(&pad);
//    u64 kHeld = padGetButtons(&pad);
//
//    if( (kDown & HidNpadButton_B and GlobalObjects::gGameBrowser.get_current_relative_depth() == 0)
//        or GlobalObjects::is_quit_now_triggered()
//        ){ // back
//      break;
//    }
//
//    GlobalObjects::gGameBrowser.scanInputs(kDown, kHeld);
//
//  } // while

  consoleExit(nullptr);

}
