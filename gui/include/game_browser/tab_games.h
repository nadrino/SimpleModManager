//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_GAMES_H
#define SIMPLEMODMANAGER_TAB_GAMES_H

#include <borealis.hpp>

class tab_games : public brls::List {

public:
  tab_games();

private:
  std::vector<brls::ListItem*> _games_list_;
  std::vector<int> _nb_mods_list_;

};


#endif //SIMPLEMODMANAGER_TAB_GAMES_H
