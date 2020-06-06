//
// Created by Nadrino on 20/05/2020.
//

#ifndef SIMPLEMODMANAGER_TEST_BLOCK_H
#define SIMPLEMODMANAGER_TEST_BLOCK_H

#include <display_element.h>
#include <SDL2/SDL_ttf.h>

#include <switch/types.h>

namespace draw{

  class test_block : public display_element {

  public:
    test_block();
    ~test_block() override;

    void reset();
    void initialize();

    void setFontSize(u32 fontSize);
    void setTextFont(TTF_Font *textFont);

    void setText(std::string text);

    void setTextColor(SDL_Color textColor);

    void draw(SDL_Renderer* sdl_render_) override;

  protected:
    void load_font();


  private:
    bool _is_initialized_;

    std::string _text_;

    u32 _font_size_;
    TTF_Font* _text_font_;
    SDL_Color _text_color_;

  };

}




#endif //SIMPLEMODMANAGER_TEST_BLOCK_H
