//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <TabGeneralSettings.h>
#include <GlobalObjects.h>
#include "FrameRoot.h"

#include "GenericToolbox.h"
#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabGeneralSettings]");
});

TabGeneralSettings::TabGeneralSettings(FrameRoot* owner_) : _owner_(owner_) {
  LogWarning << "Building general settings tab..." << std::endl;
  this->rebuildLayout();
  LogInfo << "General settings tab build." << std::endl;
}

void TabGeneralSettings::rebuildLayout() {

  itemInstallLocationPreset = new brls::ListItem(
    "\uE255 Install location:",
    "Specify from which base folder mods will be installed.\n"\
    "- If you are using Atmosphere, mods have to be installed in /atmosphere/. "\
    "This corresponds to the \"default\" preset. You need to take this path into account in your mod tree structure.\n"\
    "- If you want to set a specific install folder for a given game, please refer to its Option tab and go to \"Attribute a config preset\".",
    ""
  );
  itemInstallLocationPreset->setValue(this->getConfig().getCurrentPresetName() );
  itemInstallLocationPreset->getClickEvent()->subscribe([this](View* view) {
    brls::ValueSelectedEvent::Callback valueCallback = [this](int result) {
      if (result == -1) {
        return;
      }

      this->getConfig().setSelectedPresetIndex( result );
      _owner_->getGuiModManager().getGameBrowser().getConfigHandler().dumpConfigToFile();
      LogInfo << "Selected: " << this->getConfig().getCurrentPreset().name << " -> " << this->getConfig().getCurrentPreset().installBaseFolder << std::endl;
      this->itemInstallLocationPreset->setValue(this->getConfig().getCurrentPresetName() );
    };

    std::vector<std::string> presetNameList;
    presetNameList.reserve( this->getConfig().presetList.size() );
    for( auto& preset : this->getConfig().presetList ){
      presetNameList.emplace_back( preset.name + " \uE090 \"" + preset.installBaseFolder + "\"" );
    }

    brls::Dropdown::open(
      "Current install preset:",
      presetNameList,
      valueCallback,
      this->getConfig().selectedPresetIndex
    );
  });
  this->addView(itemInstallLocationPreset);

  auto* itemStoredModsBaseFolder = new brls::ListItem(
    "\uE431 Stored mods location:",
    "This is the place where SimpleModManager will look for your mods. From this folder, the tree structure must look like this:\n"\
    "./<name-of-the-game-or-category>/<mod-name>/<mod-tree-structure>.",
    "");
  itemStoredModsBaseFolder->setValue( this->getConfig().baseFolder );
  this->addView(itemStoredModsBaseFolder);

  auto* itemUseUI = new brls::ListItem("\uE072 Disable the GUI", "If you want to go back on the old UI, select this option.");

  itemUseUI->updateActionHint(brls::Key::B, "Back");
  itemUseUI->registerAction("Select", brls::Key::A, [&](){

    auto* dialog = new brls::Dialog("Do you want to disable this GUI and switch back to the console-style UI ?");

    dialog->addButton("Yes", [&](brls::View* view) {
      this->getConfig().useGui = false;
      brls::Application::quit();
//      GlobalObjects::setTriggerSwitchUI(true);
//      GlobalObjects::disable_cout_redirection();
    });
    dialog->addButton("No", [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

    return true;
  });
  this->addView(itemUseUI);


}

const ConfigHolder& TabGeneralSettings::getConfig() const{
  return _owner_->getGuiModManager().getGameBrowser().getConfigHandler().getConfig();
}
ConfigHolder& TabGeneralSettings::getConfig(){
  return _owner_->getGuiModManager().getGameBrowser().getConfigHandler().getConfig();
}
