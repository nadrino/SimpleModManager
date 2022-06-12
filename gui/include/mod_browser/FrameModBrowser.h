//
// Created by Adrien BLANCHET on 21/06/2020.
//

#ifndef SIMPLEMODMANAGER_FRAMEMODBROWSER_H
#define SIMPLEMODMANAGER_FRAMEMODBROWSER_H

#include <borealis.hpp>
#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModOptions.h>
#include <TabModPresets.h>

class FrameModBrowser : public brls::TabFrame {

public:
  explicit FrameModBrowser(std::string folder_);

  bool onCancel() override;

  uint8_t *getIcon();
  std::string getTitleid();

private:
  TabModBrowser* _tabModBrowser_;
  TabModOptions* _tabModOptions_;
  TabModPresets* _tabModPresets_;
  TabModPlugins* _tabModPlugins_;

  uint8_t* _icon_;
  std::string _titleid_;

};


#endif //SIMPLEMODMANAGER_FRAMEMODBROWSER_H
