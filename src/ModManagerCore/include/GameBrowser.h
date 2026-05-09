//
// Created by Nadrino on 03/09/2019.
//

#ifndef SWITCHTEMPLATE_BROWSER_H
#define SWITCHTEMPLATE_BROWSER_H

#include <Selector.h>
#include <ModManager.h>
#include <ConfigHandler.h>
#include <ModsPresetHandler.h>

#include <switch.h>

#include <string>


class GameBrowser{

public:
  GameBrowser();

  void setIsGameSelected(bool isGameSelected);

  // getters
  bool isGameSelected() const;
  const ConfigHandler &getConfigHandler() const;
  const Selector &getSelector() const;
  Selector &getSelector();
  ModManager &getModManager();
  ModsPresetHandler &getModPresetHandler();
  ConfigHandler &getConfigHandler();

  // browse
  void selectGame(const std::string &gameName_);

  // IO
  void scanInputs(u64 kDown, u64 kHeld);
  void printTerminal();
  void rebuildSelectorMenu();
  bool refreshGameList(bool force_ = false);
  std::string refreshGameListTag(const std::string& gameName_);

  // utils -> move to gui lib??
  uint8_t* getFolderIcon(const std::string& gameFolder_);

protected:
  void init();

private:
  [[nodiscard]] std::string buildGameListSignature() const;

  bool _isGameSelected_{false};
  bool _gameListReady_{false};
  std::string _gameListSignature_{};

  Selector _selector_;
  ModManager _modManager_{this};
  ConfigHandler _configHandler_;
  ModsPresetHandler _modPresetHandler_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
