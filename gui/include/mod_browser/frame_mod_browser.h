//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAME_MOD_BROWSER_H
#define SIMPLEMODMANAGER_FRAME_MOD_BROWSER_H

#include <borealis.hpp>
#include <tab_mod_browser.h>
#include <tab_mod_plugins.h>
#include <tab_mod_options.h>
#include <tab_mod_presets.h>

class frame_mod_browser : public brls::TabFrame {

public:
  explicit frame_mod_browser(std::string folder_);

  bool onCancel() override;

  uint8_t *getIcon();
  std::string getTitleid();

private:
  tab_mod_browser* _tabModBrowser_;
  tab_mod_options* _tabModOptions_;
  tab_mod_presets* _tabModPresets_;
  tab_mod_plugins* _tabModPlugins_;

  uint8_t* _icon_;
  std::string _titleid_;

};


#endif //SIMPLEMODMANAGER_FRAME_MOD_BROWSER_H
