

#include <toolbox.h> // keep it for debug

#include <borealis.hpp>
#include <GlobalObjects.h>

bool __is_new_version__;
void run_gui();
void run_console();


// MAIN
int main(int argc, char **argv){

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

  return EXIT_SUCCESS;

}

static void errorCallback(int errorCode, const char* description)
{
  brls::Logger::error("[GLFW:%d] %s", errorCode, description);
}

void run_gui(){

  // Init the app
  brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

  glfwSetErrorCallback(errorCallback);
  glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
  if (!glfwInit())
  {
//    Logger::error("Failed to initialize glfw");
//    return false;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
  glfwCreateWindow(1280, 720, "SimpleModManager", nullptr, nullptr);
  exit(EXIT_SUCCESS);
//
//  if (!brls::Application::init("SimpleModManager")){
//    brls::Logger::error("Unable to init Borealis application");
//    exit(EXIT_FAILURE);
//  }

  // Create a sample view
//  auto* rootFrame = new brls::TabFrame();
//  rootFrame->setTitle("SimpleModManager");
//  rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));

  // Add the root view to the stack
//  brls::Application::pushView(rootFrame);

//  while (brls::Application::mainLoop()){
    // do something ?
//  }

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