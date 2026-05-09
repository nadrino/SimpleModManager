//
// Import mods from a PC into sdmc:/mods via the in-app HTTP upload server.
//

#ifndef SIMPLEMODMANAGER_TABIMPORTMOD_H
#define SIMPLEMODMANAGER_TABIMPORTMOD_H

#include <borealis.hpp>

#include <string>

class TabImportMod : public brls::List {

public:
  TabImportMod();
  ~TabImportMod() override = default;

  void draw( NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx ) override;

  View* getDefaultFocus() override;

private:
  void refreshStatusLine();

  brls::Label* _bodyLabel_{ nullptr };
  brls::Label* _statusLabel_{ nullptr };
  brls::ListItem* _actionRow_{ nullptr };

  std::string _lastStatus_{};
  int _statusTick_{ 0 };
};

#endif
