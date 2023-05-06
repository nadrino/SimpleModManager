//
// Created by Adrien Blanchet on 25/04/2023.
//

#ifndef SIMPLEMODMANAGER_GUIUTILS_H
#define SIMPLEMODMANAGER_GUIUTILS_H


#include "GenericToolbox.Switch.h"

#include "switch.h"

namespace ModManagerUtils {

  inline uint8_t* getFolderIcon(const std::string& gameFolder_){
    return GenericToolbox::Switch::Utils::getIconFromTitleId(
        GenericToolbox::Switch::Utils::lookForTidInSubFolders(gameFolder_, 5)
    );
    // 5 depth as for normal mods installed in the root of the SD card: /mods/MyGame[0]/MyMod[1]/atmosphere[2]/contents[3]/0100152000022000[4]
  }

}



#endif //SIMPLEMODMANAGER_GUIUTILS_H
