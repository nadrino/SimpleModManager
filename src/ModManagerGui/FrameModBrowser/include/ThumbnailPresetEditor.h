//
// Created by Adrien BLANCHET on 30/06/2020.
//

#ifndef SIMPLEMODMANAGER_THUMBNAILPRESETEDITOR_H
#define SIMPLEMODMANAGER_THUMBNAILPRESETEDITOR_H

#include "ModsPresetHandler.h"

#include <borealis.hpp>

#include "string"
#include "vector"


class FrameModBrowser;

class ThumbnailPresetEditor : public brls::ThumbnailFrame {

public:
  explicit ThumbnailPresetEditor(FrameModBrowser* owner_, const std::string& presetName_ = "");

  void updateTags();
  void save();
  void autoAssignPresetName();

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

private:
  FrameModBrowser* _owner_{nullptr};

  PresetData _bufferPreset_;

//  std::string _presetName_{"new-preset"};
//  std::vector<std::string> _selectedModsList_;

  std::vector<brls::ListItem*> _availableModItemList_;


};


#endif //SIMPLEMODMANAGER_THUMBNAILPRESETEDITOR_H
