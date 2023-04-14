//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABGENERALSETTINGS_H
#define SIMPLEMODMANAGER_TABGENERALSETTINGS_H

#include <borealis.hpp>

class TabGeneralSettings : public brls::List {

public:
  TabGeneralSettings();

  void rebuildLayout();

  brls::ListItem* itemCurrentInstallPreset;

};


#endif //SIMPLEMODMANAGER_TABGENERALSETTINGS_H
