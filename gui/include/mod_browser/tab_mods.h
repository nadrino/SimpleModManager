//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_TAB_MODS_H
#define SIMPLEMODMANAGER_TAB_MODS_H

#include <borealis.hpp>

class tab_mods : public brls::TabFrame {

public:
  explicit tab_mods(std::string folder_);

  bool onCancel() override;

private:

  std::vector<brls::View*> _mods_list_;


};


#endif //SIMPLEMODMANAGER_TAB_MODS_H
