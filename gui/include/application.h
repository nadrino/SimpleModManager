//
// Created by Adrien BLANCHET on 21/05/2020.
//

#ifndef SIMPLEMODMANAGER_APPLICATION_H
#define SIMPLEMODMANAGER_APPLICATION_H

#include <mod_browser.h>

#include <main_layout.h>

#include <pu/Plutonium>



class application : public pu::ui::Application {

  enum app_state{
    GAME_BROWSER = 0,
    MOD_BROWSER,
    PRESET_MENU
  };

public:

  using Application::Application;
  PU_SMART_CTOR(application)

  void reset();

  // We need to define this, and use it to initialize everything
  void OnLoad() override;

private:

  // Layout instance
  main_layout::Ref layout;

  mod_browser _mod_browser_;

  app_state _current_app_state_;


};


#endif //SIMPLEMODMANAGER_APPLICATION_H
