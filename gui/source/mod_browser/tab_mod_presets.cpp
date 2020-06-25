//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "tab_mod_presets.h"
#include <borealis.hpp>
#include <GlobalObjects.h>

tab_mod_presets::tab_mod_presets() {

  auto presets_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_presets_list();
  for(const auto& preset_name : presets_list){
    auto mods_list = GlobalObjects::get_mod_browser().get_mods_preseter().get_mods_list(preset_name);
    std::string description;
    for( int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++ ){
      description += std::to_string(i_mod+1) + ": " + mods_list[i_mod];
      if(i_mod < int(mods_list.size())-1) description += "\n";
    }
    auto* item = new brls::ListItem(
      preset_name,
      "",
      ""
      );
    item->setValue(std::to_string(mods_list.size()) + " mods in this set.");
    this->addView(item);
  }

  auto* item = new brls::ListItem("\uE402 Create a mod preset");
  this->addView(item);

}
