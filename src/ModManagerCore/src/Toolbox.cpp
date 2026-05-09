//
// Created by Nadrino on 04/09/2019.
//

#include <Toolbox.h>
#include <version_config.h>

#include "GenericToolbox.Fs.h"
#include "GenericToolbox.String.h"
#include "GenericToolbox.Switch.h"
#include "Logger.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <sys/stat.h>


LoggerInit( [] {
  Logger::setUserHeaderStr( "[Toolbox]" );
} );

namespace {

constexpr const char* kTitleIdMetadataFile = ".smm_title_id";
constexpr size_t kSwitchIconBufferSize = 0x20000;

std::map<std::string, std::vector<uint8_t>> g_gameIconCache;
std::map<std::string, bool> g_gameIconAvailabilityCache;

std::string trimFolderName(std::string name) {
  while( !name.empty() && ( name.front() == ' ' || name.front() == '.' ) ) {
    name.erase( name.begin() );
  }
  while( !name.empty() && ( name.back() == ' ' || name.back() == '.' ) ) {
    name.pop_back();
  }
  return name;
}

std::string toUpperAscii(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return value;
}

bool isWindowsReservedName(const std::string& name) {
  const std::string upper = toUpperAscii(name);
  if( upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL" ) {
    return true;
  }
  if( upper.size() == 4 && ( upper.rfind("COM", 0) == 0 || upper.rfind("LPT", 0) == 0 )
      && upper[3] >= '1' && upper[3] <= '9' ) {
    return true;
  }
  return false;
}

bool isLikelyRetailGameTitleId(u64 titleId) {
  constexpr u64 kApplicationPrefixMask = 0xFF00000000000000ULL;
  constexpr u64 kRetailApplicationPrefix = 0x0100000000000000ULL;
  constexpr u64 kSystemApplicationMax = 0x010000000000FFFFULL;

  if( ( titleId & kApplicationPrefixMask ) != kRetailApplicationPrefix ) {
    return false;
  }
  if( titleId <= kSystemApplicationMax ) {
    return false;
  }

  // Base game application IDs normally end in 000. Updates/DLC/system/homebrew
  // applets/forwarders often land outside this shape and should not get mod folders.
  return ( titleId & 0xFFFULL ) == 0;
}

std::string formatTitleId(u64 titleId) {
  char out[17]{};
  std::snprintf(out, sizeof(out), "%016llX", static_cast<unsigned long long>(titleId));
  return out;
}

std::string sanitizeGameFolderName(const std::string& rawName, u64 titleId) {
  std::string out;
  out.reserve(rawName.size());

  bool lastWasSpace = false;
  for( unsigned char c : rawName ) {
    const bool invalidPathChar = c < 0x20
        || c == '<' || c == '>' || c == ':'
        || c == '"' || c == '/' || c == '\\'
        || c == '|' || c == '?' || c == '*';
    char next = invalidPathChar ? ' ' : static_cast<char>(c);
    if( next == '\t' ) {
      next = ' ';
    }
    if( next == ' ' ) {
      if( lastWasSpace ) {
        continue;
      }
      lastWasSpace = true;
    }
    else {
      lastWasSpace = false;
    }
    out.push_back(next);
  }

  out = trimFolderName(out);
  if( out.empty() ) {
    out = formatTitleId(titleId);
  }
  if( isWindowsReservedName(out) ) {
    out += "_";
  }
  return out;
}

bool hasApplicationContentMeta(u64 titleId) {
  s32 totalMeta = 0;
  const Result countRc = nsCountApplicationContentMeta(titleId, &totalMeta);
  if( R_FAILED(countRc) ) {
    LogWarning << "Could not count content meta for title " << formatTitleId(titleId)
               << ": 0x" << std::hex << countRc << std::dec << std::endl;
    return true;
  }
  if( totalMeta <= 0 ) {
    return false;
  }

  constexpr s32 batchSize = 16;
  std::vector<NsApplicationContentMetaStatus> statuses(batchSize);
  for( s32 offset = 0; offset < totalMeta; ) {
    s32 entryCount = 0;
    const Result listRc = nsListApplicationContentMetaStatus(
        titleId,
        offset,
        statuses.data(),
        batchSize,
        &entryCount
    );
    if( R_FAILED(listRc) ) {
      LogWarning << "Could not list content meta for title " << formatTitleId(titleId)
                 << ": 0x" << std::hex << listRc << std::dec << std::endl;
      return true;
    }
    if( entryCount <= 0 ) {
      break;
    }

    for( s32 i = 0; i < entryCount; ++i ) {
      if( statuses[i].meta_type == NcmContentMetaType_Application
          && statuses[i].application_id == titleId ) {
        return true;
      }
    }

    offset += entryCount;
  }

  return false;
}

std::string cleanTitleIdText(std::string titleId) {
  titleId.erase(
      std::remove_if(titleId.begin(), titleId.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
      }),
      titleId.end()
  );
  titleId = toUpperAscii(titleId);
  if( titleId.size() != 16 ) {
    return {};
  }
  for( char c : titleId ) {
    if( !std::isxdigit(static_cast<unsigned char>(c)) ) {
      return {};
    }
  }
  return titleId;
}

std::string getTitleIdMetadataPath(const std::string& gameFolderPath) {
  return GenericToolbox::joinPath(gameFolderPath, kTitleIdMetadataFile);
}

void writeGameFolderTitleIdMetadata(const std::string& gameFolderPath, u64 titleId) {
  const std::string metadataPath = getTitleIdMetadataPath(gameFolderPath);
  const std::string titleIdText = formatTitleId(titleId);
  if( cleanTitleIdText(GenericToolbox::dumpFileAsString(metadataPath)) == titleIdText ) {
    return;
  }
  GenericToolbox::dumpStringInFile(metadataPath, titleIdText + "\n");
}

uint8_t* copyCachedIcon(const std::vector<uint8_t>& cachedIcon) {
  if( cachedIcon.empty() ) {
    return nullptr;
  }

  auto* out = new uint8_t[cachedIcon.size()];
  std::memcpy(out, cachedIcon.data(), cachedIcon.size());
  return out;
}

std::string getApplicationName(NsApplicationControlData& controlData) {
  NacpLanguageEntry* langEntry = nullptr;
  if( R_SUCCEEDED(nsGetApplicationDesiredLanguage(&controlData.nacp, &langEntry))
      && langEntry != nullptr
      && langEntry->name[0] != '\0' ) {
    return langEntry->name;
  }

  for( auto& entry : controlData.nacp.lang ) {
    if( entry.name[0] != '\0' ) {
      return entry.name;
    }
  }
  return {};
}

std::string getApplicationAuthor(NsApplicationControlData& controlData) {
  NacpLanguageEntry* langEntry = nullptr;
  if( R_SUCCEEDED(nsGetApplicationDesiredLanguage(&controlData.nacp, &langEntry))
      && langEntry != nullptr
      && langEntry->author[0] != '\0' ) {
    return langEntry->author;
  }

  for( auto& entry : controlData.nacp.lang ) {
    if( entry.author[0] != '\0' ) {
      return entry.author;
    }
  }
  return {};
}

bool isKnownHomebrewTitle(const std::string& name, const std::string& author) {
  const std::string haystack = GenericToolbox::toLowerCase(name + " " + author);
  const std::vector<std::string> patterns{
      "homebrew",
      "hbmenu",
      "switchbrew",
      "devkitpro",
      "simplemodmanager",
      "tinfoil",
      "goldleaf",
      "dbi",
      "awoo installer",
      "tinwoo",
      "checkpoint",
      "edizon",
      "nx-shell",
      "ftpd",
      "daybreak"
  };

  for( const auto& pattern : patterns ) {
    if( haystack.find(pattern) != std::string::npos ) {
      return true;
    }
  }
  return false;
}

} // namespace

namespace Toolbox{
  //! External function
  std::string getAppVersion(){
    std::stringstream ss;
    ss << get_version_major() << "." << get_version_minor() << "." << get_version_micro() << get_version_tag();
    return ss.str();
  }

  void ensureModsRootFolder() {
    if( mkdir( "sdmc:/mods", 0777 ) != 0 and errno != EEXIST ) {
      LogError << "Could not create sdmc:/mods: " << std::strerror( errno ) << std::endl;
    }
  }

  std::string getGameFolderTitleId(const std::string& gameFolderPath_) {
    std::string titleId = cleanTitleIdText(
        GenericToolbox::dumpFileAsString(getTitleIdMetadataPath(gameFolderPath_))
    );
    if( !titleId.empty() ) {
      return titleId;
    }

    titleId = cleanTitleIdText(GenericToolbox::Switch::Utils::lookForTidInSubFolders(gameFolderPath_));
    if( !titleId.empty() ) {
      return titleId;
    }

    return {};
  }

  uint8_t* getGameFolderIcon(const std::string& gameFolderPath_) {
    const std::string titleId = getGameFolderTitleId(gameFolderPath_);
    if( titleId.empty() ) {
      return nullptr;
    }

    auto cacheIt = g_gameIconCache.find(titleId);
    if( cacheIt != g_gameIconCache.end() ) {
      return copyCachedIcon(cacheIt->second);
    }

    auto availabilityIt = g_gameIconAvailabilityCache.find(titleId);
    if( availabilityIt != g_gameIconAvailabilityCache.end() and not availabilityIt->second ) {
      return nullptr;
    }

    auto* icon = GenericToolbox::Switch::Utils::getIconFromTitleId(titleId);
    if( icon == nullptr ) {
      g_gameIconAvailabilityCache[titleId] = false;
      return nullptr;
    }

    g_gameIconAvailabilityCache[titleId] = true;
    g_gameIconCache[titleId] = std::vector<uint8_t>(icon, icon + kSwitchIconBufferSize);
    return icon;
  }

  bool hasGameFolderIcon(const std::string& gameFolderPath_) {
    const std::string titleId = getGameFolderTitleId(gameFolderPath_);
    if( titleId.empty() ) {
      return false;
    }

    auto availabilityIt = g_gameIconAvailabilityCache.find(titleId);
    if( availabilityIt != g_gameIconAvailabilityCache.end() ) {
      return availabilityIt->second;
    }

    auto* icon = getGameFolderIcon(gameFolderPath_);
    if( icon == nullptr ) {
      return false;
    }
    delete[] icon;
    return true;
  }

  void ensureInstalledGameModFolders(const std::string& modsRootFolder_) {
    std::string modsRoot = modsRootFolder_.empty() ? "/mods" : modsRootFolder_;
    GenericToolbox::mkdir(modsRoot);

    constexpr s32 batchSize = 128;
    std::vector<NsApplicationRecord> records(batchSize);
    auto controlData = std::make_unique<NsApplicationControlData>();
    std::set<std::string> namesUsedThisRun;

    s32 offset = 0;
    s32 detected = 0;
    s32 created = 0;
    s32 skipped = 0;
    s32 failed = 0;

    while( true ) {
      s32 entryCount = 0;
      const Result listRc = nsListApplicationRecord(records.data(), batchSize, offset, &entryCount);
      if( R_FAILED(listRc) ) {
        LogError << "Could not list installed applications: 0x" << std::hex << listRc << std::dec << std::endl;
        break;
      }
      if( entryCount <= 0 ) {
        break;
      }

      for( s32 i = 0; i < entryCount; ++i ) {
        const u64 titleId = records[i].application_id;
        if( titleId == 0 ) {
          continue;
        }
        if( !isLikelyRetailGameTitleId(titleId) ) {
          ++skipped;
          LogDebug << "Skipped non-game title id " << formatTitleId(titleId) << std::endl;
          continue;
        }
        if( !hasApplicationContentMeta(titleId) ) {
          ++skipped;
          LogDebug << "Skipped title without application content meta " << formatTitleId(titleId) << std::endl;
          continue;
        }

        std::memset(controlData.get(), 0, sizeof(NsApplicationControlData));
        u64 actualSize = 0;
        Result controlRc = nsGetApplicationControlData(
            NsApplicationControlSource_CacheOnly,
            titleId,
            controlData.get(),
            sizeof(NsApplicationControlData),
            &actualSize
        );
        if( R_FAILED(controlRc) ) {
          controlRc = nsGetApplicationControlData(
              NsApplicationControlSource_Storage,
              titleId,
              controlData.get(),
              sizeof(NsApplicationControlData),
              &actualSize
          );
        }
        if( R_FAILED(controlRc) ) {
          LogWarning << "Could not read control data for title " << formatTitleId(titleId)
                     << ": 0x" << std::hex << controlRc << std::dec << std::endl;
          continue;
        }

        const std::string applicationName = getApplicationName(*controlData);
        const std::string applicationAuthor = getApplicationAuthor(*controlData);
        if( isKnownHomebrewTitle(applicationName, applicationAuthor) ) {
          ++skipped;
          LogDebug << "Skipped known homebrew title " << applicationName
                   << " (" << formatTitleId(titleId) << ")" << std::endl;
          continue;
        }

        auto* icon = GenericToolbox::Switch::Utils::getIconFromTitleId(formatTitleId(titleId));
        if( icon == nullptr ) {
          ++skipped;
          LogDebug << "Skipped title without icon " << applicationName
                   << " (" << formatTitleId(titleId) << ")" << std::endl;
          continue;
        }
        delete[] icon;

        std::string folderName = sanitizeGameFolderName(applicationName, titleId);
        if( namesUsedThisRun.count(toUpperAscii(folderName)) != 0 ) {
          folderName += " [" + formatTitleId(titleId) + "]";
        }
        namesUsedThisRun.insert(toUpperAscii(folderName));

        const std::string folderPath = GenericToolbox::joinPath(modsRoot, folderName);
        const bool alreadyExists = GenericToolbox::isDir(folderPath);
        if( GenericToolbox::mkdir(folderPath) ) {
          writeGameFolderTitleIdMetadata(folderPath, titleId);
          ++detected;
          if( !alreadyExists ) {
            ++created;
            LogInfo << "Created mod folder for " << folderName << std::endl;
          }
        }
        else {
          ++failed;
          LogWarning << "Could not create mod folder: " << folderPath << std::endl;
        }
      }

      offset += entryCount;
      if( entryCount < batchSize ) {
        break;
      }
    }

    LogInfo << "Installed game folder sync done: " << detected
            << " detected, " << created
            << " created, " << skipped
            << " skipped, " << failed << " failed." << std::endl;
  }

}
