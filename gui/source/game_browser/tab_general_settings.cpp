//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <tab_general_settings.h>
#include <GlobalObjects.h>

tab_general_settings::tab_general_settings() {

  auto* itemCurrentInstallPreset = new brls::ListItem(
    "Current Install Preset:",
    "Specify on which base folder mods will be installed.\n If you are using Atmosphere, mods have to be installed in /atmosphere/. You need to take this path into account in your mod tree structure.",
    GlobalObjects::get_mod_browser().get_mod_manager().get_install_mods_base_folder()
  );
  itemCurrentInstallPreset->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_current_config_preset_name());
  this->addView(itemCurrentInstallPreset);

  auto* itemStoredModsBaseFolder = new brls::ListItem(
    "stored-mods-base-folder:",
    "This is the place where SimpleModManager will look for your mods. From this folder, the tree structure must look like this:\n ./<name of the game or category>/<mod name>/<mod tree structure>.",
    "");
  itemStoredModsBaseFolder->setValue(GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("stored-mods-base-folder"));
  this->addView(itemStoredModsBaseFolder);

}
