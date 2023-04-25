//
// Created by Adrien Blanchet on 18/04/2023.
//

#ifndef SIMPLEMODMANAGER_OVERLAYGUILOADER_H
#define SIMPLEMODMANAGER_OVERLAYGUILOADER_H

#include "tesla.hpp"


class OverlayGuiLoader : public tsl::Overlay {

public:
  std::unique_ptr<tsl::Gui> loadInitialGui() override;

  void initServices() override;
  void exitServices() override;

  void onShow() override;
  void onHide() override;

};

#include "implementation/OverlayGui.impl.h"

#endif //SIMPLEMODMANAGER_OVERLAYGUILOADER_H
