//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAMEROOT_H
#define SIMPLEMODMANAGER_FRAMEROOT_H

#include <borealis.hpp>

class FrameRoot : public brls::TabFrame {

public:
  FrameRoot();

  bool onCancel() override;

};


#endif //SIMPLEMODMANAGER_FRAMEROOT_H
