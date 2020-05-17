//
// Created by Adrien BLANCHET on 16/05/2020.
//

#include <switch.h>

#include <application_handler.h>


application_handler::application_handler() {

  _is_initialized_ = false;
  reset();

}

application_handler::~application_handler() {

  reset();

}

void application_handler::reset() {

  if(_is_initialized_){

    _is_initialized_ = false;

    appletExit();
    romfsExit();
    hidExit();
    nsExit();
    setsysExit();
    setExit();
    accountExit();

  }

  _break_main_loop_ = false;
  _app_render_handler_.reset();

}

void application_handler::initialize() {

  if(not _is_initialized_){

    appletInitialize();
    romfsInit();
    hidInitialize();
    nsInitialize();
    setsysInitialize();
    setInitialize();
    accountInitialize(AccountServiceType_System);

    _app_render_handler_.initialize();

    _is_initialized_ = true;

  }

}

void application_handler::start_main_loop() {

  while(not _break_main_loop_){
    _app_render_handler_.initialize_frame();
    scan_inputs();

    _app_render_handler_.finalize_frame();
  }

}

void application_handler::scan_inputs() {

  hidScanInput();
  u64 d = hidKeysDown(CONTROLLER_P1_AUTO);
  u64 h = hidKeysHeld(CONTROLLER_P1_AUTO);

  if(d & KEY_PLUS){
    _break_main_loop_ = true;
  }

}
