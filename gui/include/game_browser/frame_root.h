//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAME_ROOT_H
#define SIMPLEMODMANAGER_FRAME_ROOT_H

#include <borealis.hpp>

class frame_root : public brls::TabFrame {

public:
  frame_root();

  bool onCancel() override;

};


#endif //SIMPLEMODMANAGER_FRAME_ROOT_H
