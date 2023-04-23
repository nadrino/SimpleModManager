//
// Created by Adrien BLANCHET on 22/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAMEROOT_H
#define SIMPLEMODMANAGER_FRAMEROOT_H

#include "GameBrowser.h"

#include <borealis.hpp>

class FrameRoot : public brls::TabFrame {

public:
  FrameRoot();

  bool onCancel() override;

  const GameBrowser &getGameBrowser() const;
  GameBrowser &getGameBrowser();

private:
  GameBrowser _gameBrowser_{};

};


#endif //SIMPLEMODMANAGER_FRAMEROOT_H
