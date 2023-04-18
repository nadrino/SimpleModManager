//
// Created by Adrien Blanchet on 17/04/2023.
//

#ifndef SIMPLEMODMANAGER_POPUPLOADINGBOX_H
#define SIMPLEMODMANAGER_POPUPLOADINGBOX_H


#include "PopupLoadingView.h"

#include "borealis.hpp"


class PopupLoadingBox {

public:
  PopupLoadingBox() = default;

  [[nodiscard]] PopupLoadingView *getLoadingView() const;

  void pushView();
  void popView();


private:
  // memory handled by brls
  brls::Dialog* _loadingBox_{nullptr};
  PopupLoadingView* _loadingView_{nullptr};

};


#endif //SIMPLEMODMANAGER_POPUPLOADINGBOX_H
