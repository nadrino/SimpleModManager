//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_BROWSER_H
#define SIMPLEMODMANAGER_TAB_BROWSER_H

#include <borealis.hpp>

class tab_browser : public brls::List {

public:
  tab_browser();

private:
  std::vector<brls::ListItem*> _games_list_;
  std::vector<int> _nb_mods_list_;

};


#endif //SIMPLEMODMANAGER_TAB_BROWSER_H
