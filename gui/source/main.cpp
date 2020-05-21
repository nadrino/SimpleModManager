

#include <toolbox.h> // keep it for debug

#include <application_handler.h>
#include <pu/Plutonium>
#include <application.h>


// MAIN
int main(int argc, char **argv){

//  appletInitialize();
//  romfsInit();
//  hidInitialize();
//  nsInitialize();
//  setsysInitialize();
//  setInitialize();
//  accountInitialize(AccountServiceType_System);
//  plInitialize(PlServiceType_System);
//  pmshellInitialize();
//
//  application_handler app;
//  app.initialize();
//  app.start_main_loop();
//  app.reset();
//
//  pmshellExit();
//  plExit();
//  accountExit();
//  setExit();
//  setsysExit();
//  nsExit();
//  hidExit();
//  romfsExit();
//  appletExit();

  toolbox::set_use_embedded_switch_fs(true);

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



  return EXIT_SUCCESS;

}