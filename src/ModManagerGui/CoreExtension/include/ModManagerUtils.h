//
// Created by Adrien Blanchet on 25/04/2023.
//

#ifndef SIMPLEMODMANAGER_MODMANAGERUTILS_H
#define SIMPLEMODMANAGER_MODMANAGERUTILS_H

#include "switch.h"

namespace ModManagerUtils {

  uint8_t* getFolderIcon(const std::string& gameFolder_){
    return GenericToolbox::Switch::Utils::getFolderIconFromTitleId(
        GenericToolbox::Switch::Utils::lookForTidInSubFolders(gameFolder_)
        );
  }

}



#endif //SIMPLEMODMANAGER_MODMANAGERUTILS_H
