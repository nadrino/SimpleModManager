//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_MOD_BROWSER_H
#define SIMPLEMODMANAGER_TAB_MOD_BROWSER_H

#include <borealis.hpp>

class tab_mod_browser : public brls::List {

public:
  tab_mod_browser();

  void updateModsStatus();

private:
  std::vector<brls::ListItem*> _mods_list_;


};


#endif //SIMPLEMODMANAGER_TAB_MOD_BROWSER_H
