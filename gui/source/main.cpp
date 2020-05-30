

#include <toolbox.h> // keep it for debug

#include <application_handler.h>
#include <pu/Plutonium>
#include <application.h>

#include <mod_browser.h>

bool __is_new_version__;
mod_browser __mod_browser__;

void run_gui();
void run_console();


// MAIN
int main(int argc, char **argv){

  toolbox::enableEmbeddedSwitchFS();

  int max_depth = 1; // could be a parameter in the future

  __mod_browser__.set_only_show_folders(true);
  __mod_browser__.set_max_relative_depth(max_depth);
  __mod_browser__.initialize();

  __is_new_version__ = false;
  int last_version = std::stoi(
    toolbox::join_vector_string(
      toolbox::split_string(
        __mod_browser__.get_parameters_handler().get_parameter("last-program-version"),
        "."
        ),
      ""
    )
  );
  toolbox::make_pause();
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


  if(bool(std::stoi(__mod_browser__.get_parameters_handler().get_parameter("use-gui")))){
    run_gui();
  }
  else{
    run_console();
  }

  toolbox::disableEmbeddedSwitchFS();

  return EXIT_SUCCESS;

}

void run_gui(){

  auto renderer = pu::ui::render::Renderer::New(
    pu::ui::render::RendererInitOptions( SDL_INIT_EVERYTHING, pu::ui::render::RendererHardwareFlags )
      .WithIMG(pu::ui::render::IMGAllFlags)
      .WithMixer(pu::ui::render::MixerAllFlags)
      .WithTTF()
  );

  // Create our main application from the renderer
  auto app = application::New(renderer);
  app->reset();

  // Prepare out application. This MUST be called or Show() will exit and nothing will be rendered.
  app->Prepare();

  // Show -> start rendering in an "infinite" loop
  // If wou would like to show with a "fade in" from black-screen to the UI, use instead ->ShowWithFadeIn();
  app->ShowWithFadeIn();

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
    toolbox::ask_question("To continue, press A.", {"Ok"}, false);
  }

  __mod_browser__.print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if(kDown & KEY_B and __mod_browser__.get_current_relative_depth() == 0){ // back
      break;
    }

    __mod_browser__.scan_inputs(kDown, kHeld);

  } // while

}