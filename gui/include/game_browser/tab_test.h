//
// Created by Adrien BLANCHET on 23/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_TEST_H
#define SIMPLEMODMANAGER_TAB_TEST_H

#include <borealis.hpp>

class tab_test : public brls::List{

public:
  tab_test(int page);

  View* getDefaultFocus() override{
    return nullptr;
  }



};


#endif //SIMPLEMODMANAGER_TAB_TEST_H
