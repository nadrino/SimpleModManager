//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <tab_general_settings.h>
#include <GlobalObjects.h>

tab_general_settings::tab_general_settings() {

  rebuild_layout();

}

void tab_general_settings::rebuild_layout() {

  itemCurrentInstallPreset = new brls::ListItem(
    "Current Install Preset:",
    "Specify on which base folder mods will be installed.\n If you are using Atmosphere, mods have to be installed in /atmosphere/. You need to take this path into account in your mod tree structure.",
    ""
  );
  itemCurrentInstallPreset->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
  itemCurrentInstallPreset->getClickEvent()->subscribe([this](View* view) {
    brls::ValueSelectedEvent::Callback valueCallback = [this](int result) {
      if (result == -1)
        return;

      GlobalObjects::get_mod_browser().change_config_preset(GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list()[result]);
      brls::Logger::debug(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name().c_str());
      brls::Logger::debug(GlobalObjects::get_mod_browser().get_mod_manager().get_install_mods_base_folder().c_str());
      this->itemCurrentInstallPreset->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());

      // need to change the sublabel -> need to redraw everything

//       this->valueEvent.fire(result); // not now
    };
    brls::Dropdown::open(
      "Current Install Preset:",
      GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list(),
      valueCallback,
      GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_id()
    );
  });
  this->addView(itemCurrentInstallPreset);

  GlobalObjects::get_mod_browser().get_parameters_handler().get_presets_list();

  auto* itemStoredModsBaseFolder = new brls::ListItem(
    "stored-mods-base-folder:",
    "This is the place where SimpleModManager will look for your mods. From this folder, the tree structure must look like this:\n ./<name of the game or category>/<mod name>/<mod tree structure>.",
    "");
  itemStoredModsBaseFolder->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("stored-mods-base-folder"));
  this->addView(itemStoredModsBaseFolder);

}
