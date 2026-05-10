//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "FrameModBrowser.h"

#include <TabModBrowser.h>
#include <TabModPlugins.h>
#include <TabModPresets.h>
#include <TabModOptions.h>

#include "SystemStatusOverlay.h"

#include "GenericToolbox.Switch.h"
#include "Logger.h"

#include <sstream>
#include <vector>

LoggerInit([]{
  Logger::setUserHeaderStr("[FrameModBrowser]");
});


FrameModBrowser::FrameModBrowser(GuiModManager* guiModManagerPtr_) : _guiModManagerPtr_(guiModManagerPtr_) {

  // fetch game title
  this->setTitle( getGameBrowser().getModManager().getGameName() );

  std::string gamePath = getGameBrowser().getModManager().getGameFolderPath();

  _titleId_ = GenericToolbox::Switch::Utils::lookForTidInSubFolders( gamePath, 5);
  _icon_ = GenericToolbox::Switch::Utils::getIconFromTitleId( _titleId_ );
  if(_icon_ != nullptr){ this->setIcon(_icon_, 0x20000); }
  else{ this->setIcon("romfs:/images/icon_corner.png"); }

  this->setFooterText("SimpleModManager");


  if( not getGameBrowser().getModManager().getModList().empty() ){

    auto* parametersTabList = new brls::List();

    auto* presetParameter = new brls::ListItem("Config preset", "", "");
    presetParameter->setValue( getGameBrowser().getConfigHandler().getConfig().getCurrentPresetName() );
    parametersTabList->addView(presetParameter);

    _tabModBrowser_ = new TabModBrowser( this );
    _tabModPresets_ = new TabModPresets( this );
    _tabModOptions_ = new TabModOptions( this );
    _tabModPlugins_ = new TabModPlugins( this );

    _tabModOptions_->initialize();

    this->addTab("Mod Browser", _tabModBrowser_);
    this->addSeparator();
    this->addTab("Mod Presets", _tabModPresets_);
    this->addTab("Options", _tabModOptions_);
    this->addTab("Plugins", _tabModPlugins_);

    // Auto-recheck disabled to prevent freeze - use cache-based verification only
    // User can manually trigger verification if needed
    // if( not getGuiModManager().isBackgroundTaskRunning() ){
    //   getGuiModManager().startCheckAllModsThread();
    // }

  }
  else{
    auto* list = new brls::List();
    LogInfo("No mods found for: %s", gamePath.c_str());
    auto* item = new brls::ListItem(
        "No mods for this game are on your SD card.",
        "Put mods in: " + gamePath,
        "");
    list->addView(item);
    this->addTab("Mod Browser", list);
  }

}
void FrameModBrowser::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) {
  brls::TabFrame::draw(vg, x, y, width, height, style, ctx);
  SystemStatusOverlay::draw(vg, x, y, width, style, ctx);
  this->promptOrphanInstalledModsCleanup();
}

bool FrameModBrowser::onCancel() {

  // Go back to sidebar
  auto* lastFocus = brls::Application::getCurrentFocus();
  brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, false);

  // If the sidebar was already there, the focus has not changed
  if(lastFocus == brls::Application::getCurrentFocus()){
    LogInfo("Back on games screen...");
    brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  }
  return true;

}

uint8_t *FrameModBrowser::getIcon() {
  return _icon_;
}
std::string FrameModBrowser::getTitleId() {
  return _titleId_;
}

void FrameModBrowser::promptOrphanInstalledModsCleanup() {
  if( _orphanCleanupPromptShown_ ){
    return;
  }
  if( !this->getConfig().offerOrphanInstalledModCleanup ){
    return;
  }
  if( brls::Application::hasViewDisappearing() ){
    return;
  }
  if( brls::Application::getTopStackView() != this ){
    return;
  }
  if( this->getGuiModManager().isBackgroundTaskRunning() ){
    return;
  }

  auto& modManager = this->getGameBrowser().getModManager();
  if( !_orphanCleanupScanDone_ ){
    modManager.refreshOrphanInstalledModList();
    _orphanCleanupScanDone_ = true;
  }

  const auto& orphanMods = modManager.getOrphanInstalledModList();
  if( orphanMods.empty() ){
    return;
  }

  _orphanCleanupPromptShown_ = true;

  std::vector<std::string> modNameList;
  modNameList.reserve(orphanMods.size());
  for( const auto& orphanMod : orphanMods ){
    modNameList.emplace_back(orphanMod.modName);
  }

  std::stringstream ss;
  if( modNameList.size() == 1 ){
    if( modNameList.front() == "Unknown installed files" ){
      ss << "Installed files were found for this game, but they do not match any mod currently on your SD card. Delete these installed files?";
    }
    else{
      ss << "Installed files were found for \"" << modNameList.front()
         << "\", but this mod is no longer on your SD card. Delete these installed files?";
    }
  }
  else{
    ss << "Installed files were found for " << modNameList.size()
       << " cleanup entries that no longer match mods on your SD card. Delete these installed files?";
  }

  auto* dialog = new brls::Dialog(ss.str());
  dialog->addButton("Yes", [this, dialog, modNameList](brls::View* view) {
    dialog->close([this, modNameList]{
      this->getGuiModManager().startDeleteOrphanInstalledModsThread(modNameList);
    });
  });
  dialog->addButton("No", [dialog](brls::View* view) {
    dialog->close();
  });
  dialog->setCancelable(true);
  dialog->open();
}
