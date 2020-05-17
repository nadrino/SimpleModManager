//
// Created by Adrien BLANCHET on 16/05/2020.
//

#include "render_handler.h"

render_handler::render_handler() {

  _is_initialized_ = false;
  reset();

}

render_handler::~render_handler() {

  reset();

}

void render_handler::reset() {

  if(_is_initialized_){

    _is_initialized_ = false;

    SDL_DestroyRenderer(_sdl_renderer_);
    SDL_FreeSurface(_sdl_surface_);
    SDL_DestroyWindow(_sdl_window_);
    SDL_Quit();

  }

  _sdl_event_ = nullptr;
  _sdl_renderer_ = nullptr;
  _sdl_window_ = nullptr;
  _sdl_surface_ = nullptr;

  _background_color_.R = 255;
  _background_color_.G = 255;
  _background_color_.B = 255;
  _background_color_.A = 255;

}

void render_handler::initialize() {

  if(not _is_initialized_){

    SDL_Init(SDL_INIT_EVERYTHING);

    _sdl_window_ = SDL_CreateWindow("SDL_Window", 0, 0, 1280, 720, 0);

    _sdl_renderer_ = SDL_CreateRenderer(_sdl_window_, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    _sdl_surface_ = SDL_GetWindowSurface(_sdl_window_);

    SDL_SetRenderDrawBlendMode(_sdl_renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    _is_initialized_ = true;

  }

}

void render_handler::initialize_frame() {

  SDL_SetRenderDrawColor(_sdl_renderer_, _background_color_.R, _background_color_.G, _background_color_.B, _background_color_.A);
  SDL_RenderClear(_sdl_renderer_);

}

void render_handler::finalize_frame() {

  SDL_RenderPresent(_sdl_renderer_);

}
