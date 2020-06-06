//
// Created by Nadrino on 20/05/2020.
//

#include "test_block.h"

#include <switch.h>

#include <utility>


draw::test_block::test_block() {

  reset();

}
draw::test_block::~test_block() {

  reset();

}

void draw::test_block::reset() {

  if(_is_initialized_){

    _is_initialized_ = false;
  }

  _text_font_ = nullptr;
  _font_size_ = 23;
  _text_color_ = {255,255,255, 0};

}
void draw::test_block::initialize() {

  if(not _is_initialized_){

    load_font();

    _is_initialized_ = true;
  }

}

void draw::test_block::draw(SDL_Renderer *sdl_render_) {

  if(_is_initialized_){
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(_text_font_, _text_.c_str(), _text_color_);
    SDL_Texture *textureMessage = SDL_CreateTextureFromSurface(sdl_render_, surfaceMessage);

    int text_width = surfaceMessage->w;
    int text_height = surfaceMessage->h;

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = 50;  //controls the rect's x coordinate
    Message_rect.y = 50; // controls the rect's y coordinte
    Message_rect.w = 100; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    if(text_width <= 0){
      Message_rect.w = text_width;
    }



    SDL_RenderCopy(sdl_render_, textureMessage, nullptr, &Message_rect);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(textureMessage);
  }

}

void draw::test_block::load_font() {

  if(_text_font_ == nullptr){
    PlFontData plfont;
    plInitialize(PlServiceType_User);
    if(R_SUCCEEDED(plGetSharedFontByType(&plfont, PlSharedFontType_Standard))){
      SDL_RWops *mem = SDL_RWFromMem(plfont.address, plfont.size);
      _text_font_ = TTF_OpenFontRW(mem, 1, _font_size_);
    }
  }

}

void draw::test_block::setFontSize(u32 fontSize) {
  _font_size_ = fontSize;
}
void draw::test_block::setTextFont(TTF_Font *textFont) {
  _text_font_ = textFont;
}
void draw::test_block::setText(std::string text) {
  _text_ = std::move(text);
}
void draw::test_block::setTextColor(SDL_Color textColor) {
  _text_color_ = textColor;
}
