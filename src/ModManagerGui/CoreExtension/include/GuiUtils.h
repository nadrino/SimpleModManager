//
// Created by Adrien Blanchet on 25/04/2023.
//

#ifndef SIMPLEMODMANAGER_GUIUTILS_H
#define SIMPLEMODMANAGER_GUIUTILS_H


#include "GenericToolbox.Switch.h"

#include "switch.h"

namespace ModManagerUtils {

  inline uint8_t* getFolderIcon(const std::string& gameFolder_){
    return GenericToolbox::Switch::Utils::getFolderIconFromTitleId(
        GenericToolbox::Switch::Utils::lookForTidInSubFolders(gameFolder_)
        );
  }

}



#endif //SIMPLEMODMANAGER_GUIUTILS_H
