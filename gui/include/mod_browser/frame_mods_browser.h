//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAME_MODS_BROWSER_H
#define SIMPLEMODMANAGER_FRAME_MODS_BROWSER_H

#include <borealis.hpp>
#include <tab_mod_browser.h>
#include <tab_mod_options.h>
#include <tab_mod_presets.h>

class frame_mods_browser : public brls::TabFrame {

public:
  explicit frame_mods_browser(std::string folder_);

  bool onCancel() override;

private:
  tab_mod_browser* _tabModBrowser_;
  tab_mod_options* _tabModOptions_;
  tab_mod_presets* _tabModPresets_;

};


#endif //SIMPLEMODMANAGER_FRAME_MODS_BROWSER_H
