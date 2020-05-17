//
// Created by Adrien BLANCHET on 17/05/2020.
//

#include <color.h>

namespace draw{

  color::color() {
    R = 0;
    G = 0;
    B = 0;
    A = 255;
  }

  color::color(u8 R_, u8 G_, u8 B_, u8 A_){
    R = R_;
    G = G_;
    B = B_;
    A = A_;
  }

}