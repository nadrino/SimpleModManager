//
// Created by Adrien BLANCHET on 20/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABABOUT_H
#define SIMPLEMODMANAGER_TABABOUT_H

#include <borealis.hpp>

class TabAbout : public brls::List{

public:
  TabAbout();

  View* getDefaultFocus() override{
    return nullptr;
  }



};


#endif //SIMPLEMODMANAGER_TABABOUT_H
