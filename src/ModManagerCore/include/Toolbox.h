//
// Created by Nadrino on 04/09/2019.
//

#ifndef SIMPLEMODMANAGER_TOOLBOX_H
#define SIMPLEMODMANAGER_TOOLBOX_H

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <switch.h>

namespace Toolbox{
  //! External function
  std::string getAppVersion();

  //! Ensures sdmc:/mods exists (SD root /mods).
  void ensureModsRootFolder();

  //! Creates one folder in the mods root for each installed game title.
  void ensureInstalledGameModFolders(const std::string& modsRootFolder_ = "/mods");

  //! Gets the title id attached to a game mods folder, using SMM metadata first.
  std::string getGameFolderTitleId(const std::string& gameFolderPath_);

  //! Gets the Switch icon for a game mods folder.
  uint8_t* getGameFolderIcon(const std::string& gameFolderPath_);

  //! Tells whether a game mods folder can resolve to a Switch icon.
  bool hasGameFolderIcon(const std::string& gameFolderPath_);
}

#endif //SIMPLEMODMANAGER_TOOLBOX_H
