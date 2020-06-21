//
// Created by Adrien BLANCHET on 20/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_ABOUT_H
#define SIMPLEMODMANAGER_TAB_ABOUT_H

#include <borealis.hpp>

class tab_about : public brls::List{

public:
  tab_about();

  View* getDefaultFocus() override{
    return nullptr;
  }



};


#endif //SIMPLEMODMANAGER_TAB_ABOUT_H
