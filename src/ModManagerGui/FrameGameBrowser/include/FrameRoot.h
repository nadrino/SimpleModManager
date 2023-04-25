//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAMEROOT_H
#define SIMPLEMODMANAGER_FRAMEROOT_H

#include "GuiModManager.h"

#include <borealis.hpp>

class FrameRoot : public brls::TabFrame {

public:
  FrameRoot();

  bool onCancel() override;

  const GuiModManager &getGuiModManager() const;
  GuiModManager &getGuiModManager();

private:
  GuiModManager _guiModManager_{};

};


#endif //SIMPLEMODMANAGER_FRAMEROOT_H
