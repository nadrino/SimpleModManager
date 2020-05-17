//
// Created by Adrien BLANCHET on 17/05/2020.
//

#ifndef SIMPLEMODMANAGER_COLOR_H
#define SIMPLEMODMANAGER_COLOR_H

#include <switch.h>

namespace draw{

  struct color
  {
    color();
    color(u8 R_, u8 G_, u8 B_, u8 A_);

    u8 R;
    u8 G;
    u8 B;
    u8 A;
  };

}



#endif //SIMPLEMODMANAGER_COLOR_H
