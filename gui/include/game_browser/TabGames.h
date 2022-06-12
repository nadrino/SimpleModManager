//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TABGAMES_H
#define SIMPLEMODMANAGER_TABGAMES_H

#include <borealis.hpp>

class TabGames : public brls::List {

public:
  TabGames();

private:
  std::vector<brls::ListItem*> _games_list_;
  std::vector<int> _nb_mods_list_;

};


#endif //SIMPLEMODMANAGER_TABGAMES_H
