//
// Created by Nadrino on 20/05/2020.
//

#ifndef SIMPLEMODMANAGER_DISPLAY_ELEMENT_H
#define SIMPLEMODMANAGER_DISPLAY_ELEMENT_H

#include <string>

#include <SDL2/SDL_render.h>

namespace draw {

  class display_element {

  public:

    display_element();
    virtual ~display_element() = default;

    void set_title(std::string title_);

    virtual void draw(SDL_Renderer *sdl_renderer_) = 0;

  private:

    bool _is_visible_;

    std::string _title_;

  };

}


#endif //SIMPLEMODMANAGER_DISPLAY_ELEMENT_H
