//
// Created by Nadrino on 18/05/2020.
//

#include <rectangle.h>

namespace draw{

  rectangle::rectangle(){

    _is_initialized_ = false;
    reset();

  }

  rectangle::~rectangle() {

    reset();

  }

  void rectangle::reset() {

    if(_is_initialized_){

      _is_initialized_ = false;
    }

    _x_base_ = 0;
    _y_base_ = 0;
    _width_ = 0;
    _height_ = 0;

    _background_color_.r = 255;
    _background_color_.g = 255;
    _background_color_.b = 255;
    _background_color_.a = 255;

  }

  void rectangle::initialize() {

    _is_initialized_ = true;
  }

  void rectangle::draw(SDL_Renderer *sdl_renderer_) {

    SDL_Rect sdl_rectangle;
    sdl_rectangle.x = _x_base_;
    sdl_rectangle.y = _y_base_;
    sdl_rectangle.w = _width_;
    sdl_rectangle.h = _height_;

    SDL_SetRenderDrawColor(
      sdl_renderer_,
      _background_color_.r,
      _background_color_.g,
      _background_color_.b,
      _background_color_.a
      );

    SDL_RenderFillRect(
      sdl_renderer_,
      &sdl_rectangle
      );

  }

  void rectangle::setWidth(u32 width) {
    _width_ = width;
  }

  void rectangle::setXBase(u32 xBase) {
    _x_base_ = xBase;
  }

  void rectangle::setYBase(u32 yBase) {
    _y_base_ = yBase;
  }

  void rectangle::setHeight(u32 height) {
    _height_ = height;
  }

  void rectangle::setBackgroundColor(const SDL_Color &backgroundColor) {
    _background_color_ = backgroundColor;
  }

}
