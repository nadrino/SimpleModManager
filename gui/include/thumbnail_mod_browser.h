//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_THUMBNAIL_MOD_BROWSER_H
#define SIMPLEMODMANAGER_THUMBNAIL_MOD_BROWSER_H

#include <borealis.hpp>

class thumbnail_mod_browser : public brls::TabFrame {

public:
  explicit thumbnail_mod_browser(std::string folder_);

  bool onCancel() override;

private:


};


#endif //SIMPLEMODMANAGER_THUMBNAIL_MOD_BROWSER_H
