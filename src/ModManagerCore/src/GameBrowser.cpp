//
// Created by Nadrino on 03/09/2019.
//

#include <GameBrowser.h>
#include <Toolbox.h>


#include "GenericToolbox.Switch.h"
#include "GenericToolbox.Vector.h"

#include <switch.h>
#include <switch/services/pdm.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include <sys/stat.h>

namespace {

struct GameSortEntry{
  std::string title{};
  std::string path{};
  size_t nMods{0};
  ModStatusSummary modStatusSummary{};

  bool hasFirstModTimestamp{false};
  bool hasLastModTimestamp{false};
  u64 firstModTimestamp{0};
  u64 lastModTimestamp{0};

  bool hasPlayStats{false};
  u64 firstPlayedTimestamp{0};
  u64 lastPlayedTimestamp{0};
  u64 playtimeNs{0};
  u64 launchCount{0};
};

std::string toLowerAscii(std::string value){
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
    return static_cast<char>(std::tolower(c));
  });
  return value;
}

bool sortNeedsPlayStats(const ConfigHolder::SortGameList& sortMode){
  switch( sortMode.value ){
    case ConfigHolder::SortGameList::GameLaunched:
    case ConfigHolder::SortGameList::PlayTime:
    case ConfigHolder::SortGameList::LaunchCount:
      return true;
    default:
      return false;
  }
}

u64 parseTitleId(const std::string& titleId){
  if( titleId.empty() ){
    return 0;
  }

  u64 out{0};
  std::stringstream ss;
  ss << std::hex << titleId;
  ss >> out;
  if( ss.fail() ){
    return 0;
  }
  return out;
}

void fillModTimestamps(GameSortEntry& entry){
  auto modList = GenericToolbox::lsDirs(entry.path);
  entry.nMods = modList.size();

  for( const auto& modName : modList ){
    const std::string modPath = GenericToolbox::joinPath(entry.path, modName);
    struct stat result{};
    if( stat(modPath.c_str(), &result) != 0 ){
      continue;
    }

    const u64 timestamp = static_cast<u64>(result.st_mtime);
    if( timestamp == 0 ){
      continue;
    }

    if( not entry.hasFirstModTimestamp or timestamp < entry.firstModTimestamp ){
      entry.firstModTimestamp = timestamp;
      entry.hasFirstModTimestamp = true;
    }
    if( not entry.hasLastModTimestamp or timestamp > entry.lastModTimestamp ){
      entry.lastModTimestamp = timestamp;
      entry.hasLastModTimestamp = true;
    }
  }
}

void fillPlayStats(GameSortEntry& entry){
  const u64 titleId = parseTitleId(Toolbox::getGameFolderTitleId(entry.path));
  if( titleId == 0 ){
    return;
  }

  PdmPlayStatistics stats{};
  if( R_FAILED(pdmqryQueryPlayStatisticsByApplicationId(titleId, false, &stats)) ){
    return;
  }

  entry.firstPlayedTimestamp = stats.first_timestamp_user;
  entry.lastPlayedTimestamp = stats.last_timestamp_user;
  entry.playtimeNs = stats.playtime;
  entry.launchCount = stats.total_launches;
  entry.hasPlayStats = stats.first_timestamp_user != 0
      or stats.last_timestamp_user != 0
      or stats.playtime != 0
      or stats.total_launches != 0;
}

bool hasSortValue(const GameSortEntry& entry, const ConfigHolder& config){
  const bool ascending = config.sortGameListDirection == ConfigHolder::SortGameListDirection::Ascending;
  const auto sortMode = config.sortGameList;

  switch( sortMode.value ){
    case ConfigHolder::SortGameList::Alphabetical:
    case ConfigHolder::SortGameList::NbMods:
    case ConfigHolder::SortGameList::NoSort:
      return true;
    case ConfigHolder::SortGameList::GameLaunched:
      return entry.hasPlayStats and (ascending ? entry.firstPlayedTimestamp : entry.lastPlayedTimestamp) != 0;
    case ConfigHolder::SortGameList::ModAdded:
      return ascending ? entry.hasFirstModTimestamp : entry.hasLastModTimestamp;
    case ConfigHolder::SortGameList::PlayTime:
      return entry.hasPlayStats;
    case ConfigHolder::SortGameList::LaunchCount:
      return entry.hasPlayStats;
    default:
      return true;
  }
}

u64 getSortValue(const GameSortEntry& entry, const ConfigHolder& config){
  const bool ascending = config.sortGameListDirection == ConfigHolder::SortGameListDirection::Ascending;

  switch( config.sortGameList.value ){
    case ConfigHolder::SortGameList::NbMods: return entry.nMods;
    case ConfigHolder::SortGameList::GameLaunched: return ascending ? entry.firstPlayedTimestamp : entry.lastPlayedTimestamp;
    case ConfigHolder::SortGameList::ModAdded: return ascending ? entry.firstModTimestamp : entry.lastModTimestamp;
    case ConfigHolder::SortGameList::PlayTime: return entry.playtimeNs;
    case ConfigHolder::SortGameList::LaunchCount: return entry.launchCount;
    default: return 0;
  }
}

std::string formatTimestamp(u64 timestamp){
  if( timestamp == 0 ){
    return "Unknown";
  }

  std::time_t rawTime = static_cast<std::time_t>(timestamp);
  std::tm* timeInfo = std::localtime(&rawTime);
  if( timeInfo == nullptr ){
    return std::to_string(timestamp);
  }

  char buffer[32]{};
  if( std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeInfo) == 0 ){
    return std::to_string(timestamp);
  }
  return buffer;
}

std::string formatPlaytime(u64 playtimeNs){
  constexpr u64 nsPerMinute = 60ULL * 1000ULL * 1000ULL * 1000ULL;
  u64 totalMinutes = playtimeNs / nsPerMinute;
  if( totalMinutes == 0 ){
    return "0 min";
  }

  const u64 days = totalMinutes / (24 * 60);
  totalMinutes %= (24 * 60);
  const u64 hours = totalMinutes / 60;
  const u64 minutes = totalMinutes % 60;

  std::stringstream ss;
  if( days > 0 ){
    ss << days << "d ";
  }
  if( hours > 0 or days > 0 ){
    ss << hours << "h ";
  }
  ss << minutes << "min";
  return ss.str();
}

const PresetConfig& resolvePresetForGame(const std::string& gameFolderPath, const ConfigHolder& config){
  static PresetConfig fallbackPreset{};
  std::string presetName;
  const std::string configFilePath = GenericToolbox::joinPath(gameFolderPath, "this_folder_config.txt");
  if( GenericToolbox::isFile(configFilePath) ){
    presetName = GenericToolbox::dumpFileAsString(configFilePath);
    GenericToolbox::trimInputString(presetName, " \n\r\t");
  }

  if( !presetName.empty() ){
    for( const auto& preset : config.presetList ){
      if( preset.name == presetName ){
        return preset;
      }
    }
  }

  if( !config.presetList.empty()
      && config.selectedPresetIndex >= 0
      && config.selectedPresetIndex < int(config.presetList.size()) ){
    return config.presetList[config.selectedPresetIndex];
  }

  fallbackPreset.name = "default";
  fallbackPreset.installBaseFolder = "/atmosphere";
  return fallbackPreset;
}

std::string buildSortTag(const GameSortEntry& entry, const ConfigHolder& config){
  const std::string statusSummary = ModManager::formatGameStatusSummary(entry.modStatusSummary);
  switch( config.sortGameList.value ){
    case ConfigHolder::SortGameList::GameLaunched:
      return statusSummary + " | " + (entry.hasPlayStats ? "Game launched: " + formatTimestamp(getSortValue(entry, config)) : "Never launched");
    case ConfigHolder::SortGameList::ModAdded:
      return statusSummary + " | " + (hasSortValue(entry, config) ? "Mod added: " + formatTimestamp(getSortValue(entry, config)) : "No mods");
    case ConfigHolder::SortGameList::PlayTime:
      return statusSummary + " | " + (entry.hasPlayStats ? "Play time: " + formatPlaytime(entry.playtimeNs) : "No play data");
    case ConfigHolder::SortGameList::LaunchCount:
      return statusSummary + " | " + (entry.hasPlayStats ? "Launches: " + std::to_string(entry.launchCount) : "No launch data");
    default:
      return statusSummary;
  }
}

void sortGameEntries(std::vector<GameSortEntry>& entries, const ConfigHolder& config){
  if( config.sortGameList == ConfigHolder::SortGameList::NoSort ){
    return;
  }

  const bool ascending = config.sortGameListDirection == ConfigHolder::SortGameListDirection::Ascending;
  const auto sortMode = config.sortGameList;

  std::sort(entries.begin(), entries.end(), [&](const GameSortEntry& a, const GameSortEntry& b){
    const bool aHasValue = hasSortValue(a, config);
    const bool bHasValue = hasSortValue(b, config);
    if( aHasValue != bHasValue ){
      return aHasValue;
    }

    if( sortMode == ConfigHolder::SortGameList::Alphabetical ){
      const auto aTitle = toLowerAscii(a.title);
      const auto bTitle = toLowerAscii(b.title);
      if( aTitle != bTitle ){
        return ascending ? aTitle < bTitle : aTitle > bTitle;
      }
      return a.title < b.title;
    }

    const u64 aValue = getSortValue(a, config);
    const u64 bValue = getSortValue(b, config);
    if( aValue != bValue ){
      return ascending ? aValue < bValue : aValue > bValue;
    }

    return toLowerAscii(a.title) < toLowerAscii(b.title);
  });
}

} // namespace


GameBrowser::GameBrowser(){ this->refreshGameList(true); }

void GameBrowser::setIsGameSelected(bool isGameSelected) {
  _isGameSelected_ = isGameSelected;
}

// getters
bool GameBrowser::isGameSelected() const {
  return _isGameSelected_;
}
const ConfigHandler &GameBrowser::getConfigHandler() const {
  return _configHandler_;
}
const Selector &GameBrowser::getSelector() const{
  return _selector_;
}
Selector &GameBrowser::getSelector(){
  return _selector_;
}
ModManager &GameBrowser::getModManager(){
  return _modManager_;
}
ConfigHandler &GameBrowser::getConfigHandler(){
  return _configHandler_;
}
ModsPresetHandler &GameBrowser::getModPresetHandler(){
  return _modPresetHandler_;
}

// Browse
void GameBrowser::selectGame(const std::string &gameName_) {
  _modManager_.setGameName( gameName_ );
  _modManager_.setGameFolderPath( _configHandler_.getConfig().baseFolder + "/" + gameName_ );
  _modPresetHandler_.setModFolder( _configHandler_.getConfig().baseFolder + "/" + gameName_ );

  _isGameSelected_ = true;
}


// Terminal
void GameBrowser::scanInputs(u64 kDown, u64 kHeld){

  // nothing to do?
  if( kDown == 0 and kHeld == 0 ){ return; }

  if( _isGameSelected_ ){
    // back button pressed?
    if( kDown & HidNpadButton_B ){
      _isGameSelected_ = false;
      // will print the game browser
    }
    else{
      _modManager_.scanInputs( kDown, kHeld );
    }
    return;
  }

  // forward to the selector
  _selector_.scanInputs( kDown, kHeld );


  if     ( kDown & HidNpadButton_A ){ // select folder / apply mod
    this->selectGame( _selector_.getSelectedEntryTitle() );
  }
  else if( kDown & HidNpadButton_Y ){ // switch between config preset
    _configHandler_.selectNextPreset();
  }
  else if( kDown & HidNpadButton_ZL or kDown & HidNpadButton_ZR ){
    // switch between config preset
    auto answer = Selector::askQuestion(
        "Do you want to switch back to the GUI ?",
        std::vector<std::string>({"Yes", "No"})
    );
    if(answer == "Yes") {
      _configHandler_.getConfig().useGui = true;
      _configHandler_.dumpConfigToFile();
      consoleExit(nullptr);
      exit( EXIT_SUCCESS );
      // TODO QUIT?
//      GlobalObjects::set_quit_now_triggered(true);
    }
  }

}
void GameBrowser::printTerminal(){

  if( _isGameSelected_ ){
    _modManager_.printTerminal();
    return;
  }

  if( _selector_.getFooter().empty() ){
    // first build -> page numbering
    rebuildSelectorMenu();
  }

  // update page
  rebuildSelectorMenu();

  // print on screen
  _selector_.printTerminal();
}
void GameBrowser::rebuildSelectorMenu(){
  _selector_.clearMenu();

  _selector_.getHeader() >> "SimpleModManager v" >> Toolbox::getAppVersion() << std::endl;
  _selector_.getHeader() << GenericToolbox::ColorCodes::redBackground << "Current Folder : ";
  _selector_.getHeader() << _configHandler_.getConfig().baseFolder << std::endl;
  _selector_.getHeader() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;

  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "  Page (" << _selector_.getCursorPage() + 1 << "/" << _selector_.getNbPages() << ")" << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << "Configuration preset : " << GenericToolbox::ColorCodes::greenBackground;
  _selector_.getFooter() << _configHandler_.getConfig().getCurrentPresetName() << GenericToolbox::ColorCodes::resetColor << std::endl;
  _selector_.getFooter() << "install-mods-base-folder = " + _configHandler_.getConfig().getCurrentPreset().installBaseFolder << std::endl;
  _selector_.getFooter() << GenericToolbox::repeatString("*", GenericToolbox::getTerminalWidth()) << std::endl;
  _selector_.getFooter() << " A : Select folder" >> "Y : Change config preset " << std::endl;
  _selector_.getFooter() << " B : Quit" >> "ZL/ZR : Switch back to the GUI " << std::endl;
  _selector_.getFooter() << std::endl;
  _selector_.getFooter() << std::endl;
  _selector_.getFooter() << std::endl;

  _selector_.invalidatePageCache();
  _selector_.refillPageEntryCache();
}

bool GameBrowser::refreshGameList(bool force_){
  const std::string signature = this->buildGameListSignature();
  if( not force_ and _gameListReady_ and signature == _gameListSignature_ ){
    return false;
  }

  _selector_ = Selector();
  this->init();
  _gameListSignature_ = signature;
  _gameListReady_ = true;
  return true;
}

std::string GameBrowser::refreshGameListTag(const std::string& gameName_){
  if( gameName_.empty() ){
    return {};
  }

  GameSortEntry entry;
  entry.title = gameName_;
  entry.path = GenericToolbox::joinPath(_configHandler_.getConfig().baseFolder, gameName_);
  fillModTimestamps(entry);
  const auto& preset = resolvePresetForGame(entry.path, _configHandler_.getConfig());
  entry.modStatusSummary = ModManager::readGameStatusSummary(entry.path, preset.name);

  const bool needsPlayStats = sortNeedsPlayStats(_configHandler_.getConfig().sortGameList);
  const bool playStatsReady = not needsPlayStats or R_SUCCEEDED(pdmqryInitialize());
  if( needsPlayStats and playStatsReady ){
    fillPlayStats(entry);
    pdmqryExit();
  }

  const std::string tag = buildSortTag(entry, _configHandler_.getConfig());
  for( size_t iEntry = 0; iEntry < _selector_.getEntryList().size(); ++iEntry ){
    if( _selector_.getEntryList()[iEntry].title == gameName_ ){
      _selector_.setTag(iEntry, tag);
      break;
    }
  }

  return tag;
}

uint8_t* GameBrowser::getFolderIcon(const std::string& gameFolder_){
  if( _isGameSelected_ ){ return nullptr; }
  std::string game_folder_path = _configHandler_.getConfig().baseFolder + "/" + gameFolder_;
  return Toolbox::getGameFolderIcon(game_folder_path);
}

// protected
void GameBrowser::init(){
  auto gameList = GenericToolbox::lsDirs( _configHandler_.getConfig().baseFolder );

  std::vector<GameSortEntry> visibleGameList;
  visibleGameList.reserve( gameList.size() );

  const bool needsPlayStats = sortNeedsPlayStats(_configHandler_.getConfig().sortGameList);
  const bool playStatsReady = not needsPlayStats or R_SUCCEEDED(pdmqryInitialize());

  for( auto& game : gameList ){
    const std::string gameFolderPath = _configHandler_.getConfig().baseFolder + "/" + game;
    if( !Toolbox::hasGameFolderIcon(gameFolderPath) ){
      continue;
    }

    visibleGameList.emplace_back();
    visibleGameList.back().title = game;
    visibleGameList.back().path = gameFolderPath;
    fillModTimestamps(visibleGameList.back());
    const auto& preset = resolvePresetForGame(gameFolderPath, _configHandler_.getConfig());
    visibleGameList.back().modStatusSummary = ModManager::readGameStatusSummary(gameFolderPath, preset.name);
    if( needsPlayStats and playStatsReady ){
      fillPlayStats(visibleGameList.back());
    }
  }

  if( needsPlayStats and playStatsReady ){
    pdmqryExit();
  }

  sortGameEntries(visibleGameList, _configHandler_.getConfig());

  _selector_.getEntryList().reserve( visibleGameList.size() );
  for( const auto& game : visibleGameList ){
    _selector_.getEntryList().emplace_back();
    _selector_.getEntryList().back().title = game.title;
    _selector_.getEntryList().back().tag = buildSortTag(game, _configHandler_.getConfig());
  }
}

std::string GameBrowser::buildGameListSignature() const{
  std::stringstream ss;
  ss << _configHandler_.getConfig().baseFolder << "|";
  ss << _configHandler_.getConfig().getCurrentPresetName() << "|";
  ss << _configHandler_.getConfig().sortGameList.toString() << "|";
  ss << _configHandler_.getConfig().sortGameListDirection.toString() << "|";

  auto gameList = GenericToolbox::lsDirs( _configHandler_.getConfig().baseFolder );
  std::sort(gameList.begin(), gameList.end());

  for( const auto& game : gameList ){
    const std::string gameFolderPath = _configHandler_.getConfig().baseFolder + "/" + game;
    struct stat gameStat{};
    const auto gameMtime = stat(gameFolderPath.c_str(), &gameStat) == 0 ? gameStat.st_mtime : 0;

    ss << game << ":" << gameMtime << ":";
    const std::string cacheFilePath = GenericToolbox::joinPath(gameFolderPath, "mods_status_cache.txt");
    struct stat cacheStat{};
    if( stat(cacheFilePath.c_str(), &cacheStat) == 0 ){
      ss << "cache=" << cacheStat.st_mtime << ":" << cacheStat.st_size << ":";
    }
    const std::string customPresetPath = GenericToolbox::joinPath(gameFolderPath, "this_folder_config.txt");
    struct stat customPresetStat{};
    if( stat(customPresetPath.c_str(), &customPresetStat) == 0 ){
      ss << "custom=" << customPresetStat.st_mtime << ":" << customPresetStat.st_size << ":";
    }

    auto modList = GenericToolbox::lsDirs(gameFolderPath);
    std::sort(modList.begin(), modList.end());
    ss << modList.size() << "[";
    for( const auto& mod : modList ){
      const std::string modPath = GenericToolbox::joinPath(gameFolderPath, mod);
      struct stat modStat{};
      const auto modMtime = stat(modPath.c_str(), &modStat) == 0 ? modStat.st_mtime : 0;
      ss << mod << ":" << modMtime << ";";
    }
    ss << "]";
  }

  return ss.str();
}
