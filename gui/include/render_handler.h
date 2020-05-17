//
// Created by Adrien BLANCHET on 16/05/2020.
//

#ifndef SIMPLEMODMANAGER_RENDER_HANDLER_H
#define SIMPLEMODMANAGER_RENDER_HANDLER_H

#include <SDL2/SDL.h>

// this project
#include <color.h>

class render_handler {

public:

  render_handler();
  virtual ~render_handler();

  void reset();
  void initialize();

  void initialize_frame();
  void finalize_frame();

private:

  bool _is_initialized_;

  SDL_Event *_sdl_event_;
  SDL_Window *_sdl_window_;
  SDL_Renderer *_sdl_renderer_;
  SDL_Surface *_sdl_surface_;

  draw::color _background_color_;

};


#endif //SIMPLEMODMANAGER_RENDER_HANDLER_H
