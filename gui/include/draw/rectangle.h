//
// Created by Adrien BLANCHET on 18/05/2020.
//

#ifndef SIMPLEMODMANAGER_RECTANGLE_H
#define SIMPLEMODMANAGER_RECTANGLE_H

#include <switch.h>
#include <display_element.h>

namespace draw{

  class rectangle : public display_element {

  public:

    rectangle();
    ~rectangle() override;

    void reset();
    void initialize();

    void setWidth(u32 width);

    void setXBase(u32 xBase);

    void setYBase(u32 yBase);

    void setHeight(u32 height);

    void setBackgroundColor(const SDL_Color &backgroundColor);

    void draw(SDL_Renderer *sdl_renderer_);

  private:

    bool _is_initialized_;

    u32 _x_base_;
    u32 _y_base_;
    u32 _width_;
    u32 _height_;

    SDL_Color _background_color_;

  };

}




#endif //SIMPLEMODMANAGER_RECTANGLE_H
