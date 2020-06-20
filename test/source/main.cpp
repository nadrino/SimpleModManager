
#include <borealis.hpp>

// MAIN
int main(int argc, char **argv){

  brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

  if (!brls::Application::init("SimpleModManager")){
    brls::Logger::error("Unable to init Borealis application");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;

}