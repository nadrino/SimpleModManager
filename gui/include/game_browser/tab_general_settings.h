//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_GENERAL_SETTINGS_H
#define SIMPLEMODMANAGER_TAB_GENERAL_SETTINGS_H

#include <borealis.hpp>

class tab_general_settings : public brls::List {

public:
  tab_general_settings();

  brls::ListItem* itemCurrentInstallPreset;

};


#endif //SIMPLEMODMANAGER_TAB_GENERAL_SETTINGS_H
