//
// Created by Adrien Blanchet on 14/04/2023.
//


#include "SimpleModManager.h"

#include <FrameRoot.h>
#include "ConsoleHandler.h"

#include "ConfigHandler.h"

#include "Logger.h"

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

  ConfigHandler c;
  if( c.getConfig().useGui ){ runGui(); }
  else{
    consoleInit(nullptr);
    ConsoleHandler::run();
    consoleExit(nullptr);
  }

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
