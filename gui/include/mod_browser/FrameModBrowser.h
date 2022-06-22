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
  explicit FrameModBrowser(const std::string& folder_);

  bool onCancel() override;

  uint8_t *getIcon();
  std::string getTitleId();

private:
  TabModBrowser* _tabModBrowser_{nullptr};
  TabModOptions* _tabModOptions_{nullptr};
  TabModPresets* _tabModPresets_{nullptr};
  TabModPlugins* _tabModPlugins_{nullptr};

  uint8_t* _icon_{nullptr};
  std::string _titleId_{};

};


#endif //SIMPLEMODMANAGER_FRAMEMODBROWSER_H
