//
// Created by Adrien BLANCHET on 16/05/2020.
//

#include <switch.h>

#include <application_handler.h>
#include <draw/rectangle.h>
#include <draw/test_block.h>


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

  for(auto& element : _elements_list_) delete element;
  _elements_list_.clear();

  _current_app_state_ = GAME_BROWSER;
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
    _app_render_handler_.get_background_color().r = 80;
    _app_render_handler_.get_background_color().g = 80;
    _app_render_handler_.get_background_color().b = 80;
    _app_render_handler_.get_background_color().a = 255;

//    auto* rect = new draw::rectangle();
//    rect->setWidth(40);
//    rect->setHeight(40);
//    rect->initialize();
//    rect->set_title("rect");
//    _elements_list_.emplace_back(
//      rect
//      );

    auto* text = new draw::test_block();
    text->setText("THIS IS A TEST");
    text->initialize();
    _elements_list_.emplace_back(text);

    _is_initialized_ = true;

  }

}

void application_handler::start_main_loop() {

  while(not _break_main_loop_){
    _app_render_handler_.initialize_frame();
    scan_inputs();

    switch(_current_app_state_){

      case GAME_BROWSER:
        break;

      case MOD_BROWSER:
        break;

      case PRESET_MENU:
        break;

    }

    for(auto& element : _elements_list_){
      element->draw(_app_render_handler_.get_sdl_renderer());
    }

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

void application_handler::draw_ui() {



}
