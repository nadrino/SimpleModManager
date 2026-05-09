//
// MTP responder wrapper using vendored mtp-server-nx core.
//

#include "ModsMtpServer.h"

#include "ConfigHandler.h"
#include "Logger.h"

#include "MtpServer.h"
#include "MtpStorage.h"
#include "SwitchMtpDatabase.h"
#include "USBMtpInterface.h"
#include "usb.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <minizip/unzip.h>
#include <switch.h>

extern "C" {
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
}

LoggerInit( [] {
  Logger::setUserHeaderStr( "[ModsMtpServer]" );
} );

namespace {

std::mutex g_mutex;
std::mutex g_statusMutex;
std::mutex g_serverMutex;
std::mutex g_powerMutex;
std::thread g_thread;
std::atomic<bool> g_workerActive{ false };
std::atomic<bool> g_starting{ false };
std::atomic<bool> g_running{ false };
std::atomic<bool> g_stopping{ false };
std::atomic<bool> g_stopRequested{ false };
std::atomic<bool> g_appShuttingDown{ false };
std::atomic<bool> g_usbInitialized{ false };
std::atomic<bool> g_disconnectDetected{ false };
std::atomic<bool> g_restartRequested{ false };
std::string g_status = "MTP stopped.";
bool g_keepAwakeActive = false;
bool g_autoSleepWasDisabled = false;
const auto g_processStartTs = std::chrono::steady_clock::now();
std::atomic<long long> g_lastTransitionMs{ 0 };
std::atomic<long long> g_waitingForHostSinceMs{ 0 };
std::atomic<long long> g_pendingStartDelayMs{ 0 };
std::atomic<int> g_waitingForHostRestartCount{ 0 };

constexpr long long kMtpStartGuardAfterBootMs = 3500;
constexpr long long kMtpTransitionCooldownMs = 2000;
constexpr long long kMtpStoppingTextMaxMs = 2500;
constexpr long long kMtpStopRetryMs = 1200;
constexpr long long kMtpStartingTextMaxMs = 4500;
constexpr long long kMtpHostWaitAutoRestartMs = 4500;
constexpr int kMtpHostWaitMaxAutoRestarts = 3;
constexpr size_t kZipReadBufferSize = 64 * 1024;
constexpr const char* kModsRoot = "sdmc:/mods";

android::MtpServer* g_server = nullptr;
android::MtpStorage* g_storage = nullptr;
android::MtpDatabase* g_database = nullptr;
USBMtpInterface* g_mtpInterface = nullptr;

std::atomic<int> g_lastUsbState{ static_cast<int>( UsbState_Detached ) };

void setStatus( const std::string& s ) {
  std::lock_guard<std::mutex> lock( g_statusMutex );
  g_status = s;
}

long long nowMs() {
  const auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>( now.time_since_epoch() ).count();
}

void setMtpKeepAwake( bool enabled ) {
  std::lock_guard<std::mutex> lock( g_powerMutex );
  if( enabled ) {
    if( g_keepAwakeActive ) {
      return;
    }
    bool wasDisabled = false;
    if( R_FAILED( appletIsAutoSleepDisabled( &wasDisabled ) ) ) {
      wasDisabled = false;
    }
    g_autoSleepWasDisabled = wasDisabled;
    appletSetAutoSleepDisabled( true );
    appletSetMediaPlaybackState( true );
    g_keepAwakeActive = true;
    return;
  }

  if( !g_keepAwakeActive ) {
    return;
  }
  appletSetMediaPlaybackState( false );
  if( !g_autoSleepWasDisabled ) {
    appletSetAutoSleepDisabled( false );
  }
  g_keepAwakeActive = false;
}

long long sinceProcessStartMs() {
  const auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>( now - g_processStartTs ).count();
}

long long bootGuardRemainingMs() {
  const long long elapsed = sinceProcessStartMs();
  if( elapsed >= kMtpStartGuardAfterBootMs ) return 0;
  return kMtpStartGuardAfterBootMs - elapsed;
}

long long transitionCooldownRemainingMs() {
  const long long last = g_lastTransitionMs.load();
  if( last <= 0 ) return 0;
  const long long elapsed = nowMs() - last;
  if( elapsed >= kMtpTransitionCooldownMs ) return 0;
  return kMtpTransitionCooldownMs - elapsed;
}

void requestStop_NoLock( const char* statusMsg ) {
  g_stopping.store( true );
  g_stopRequested.store( true );
  g_lastTransitionMs.store( nowMs() );
  setStatus( statusMsg );
  std::lock_guard<std::mutex> lk( g_serverMutex );
  if( g_server != nullptr ) {
    g_server->stop();
  }
}

std::string getStatus() {
  std::lock_guard<std::mutex> lock( g_statusMutex );
  return g_status;
}

bool hasNoActiveSession() {
  return !g_workerActive.load() && !g_running.load() && !g_starting.load();
}

bool shouldRestartWhileWaitingForHost( UsbState state ) {
  (void)state;
  if( g_restartRequested.load() || g_stopRequested.load() || g_stopping.load() ) {
    return false;
  }

  const long long now = nowMs();
  const long long waitingSince = g_waitingForHostSinceMs.load();
  if( waitingSince <= 0 ) {
    g_waitingForHostSinceMs.store( now );
    return false;
  }

  if( now - waitingSince < kMtpHostWaitAutoRestartMs ) {
    return false;
  }

  if( g_waitingForHostRestartCount.load() >= kMtpHostWaitMaxAutoRestarts ) {
    return false;
  }

  g_waitingForHostRestartCount.fetch_add( 1 );
  g_waitingForHostSinceMs.store( 0 );
  g_restartRequested.store( true );
  return true;
}

bool hasSuffixIgnoreCase( const std::string& value, const char* suffix ) {
  const size_t suffixLen = std::strlen( suffix );
  if( value.size() < suffixLen ) {
    return false;
  }
  const size_t offset = value.size() - suffixLen;
  for( size_t i = 0; i < suffixLen; ++i ) {
    const char a = static_cast<char>( std::tolower( static_cast<unsigned char>( value[offset + i] ) ) );
    const char b = static_cast<char>( std::tolower( static_cast<unsigned char>( suffix[i] ) ) );
    if( a != b ) {
      return false;
    }
  }
  return true;
}

std::string sanitizeArchivePath( const std::string& raw ) {
  std::string normalized;
  normalized.reserve( raw.size() );
  for( char c : raw ) {
    normalized += ( c == '\\' ) ? '/' : c;
  }

  while( !normalized.empty() && normalized.front() == '/' ) {
    normalized.erase( normalized.begin() );
  }

  std::string out;
  size_t pos = 0;
  while( pos < normalized.size() ) {
    while( pos < normalized.size() && normalized[pos] == '/' ) {
      ++pos;
    }
    size_t end = pos;
    while( end < normalized.size() && normalized[end] != '/' ) {
      ++end;
    }
    const std::string segment = normalized.substr( pos, end - pos );
    pos = end;

    if( segment.empty() || segment == "." ) {
      continue;
    }
    if( segment == ".." || segment.find( ':' ) != std::string::npos ) {
      return {};
    }
    if( !out.empty() ) {
      out += '/';
    }
    out += segment;
  }
  return out;
}

bool ensureDirectory( const std::string& dir ) {
  if( dir.empty() ) {
    return false;
  }
  if( mkdir( dir.c_str(), 0777 ) == 0 || errno == EEXIST ) {
    return true;
  }
  return false;
}

bool ensureParentDirectories( const std::string& fullPath ) {
  const size_t slash = fullPath.rfind( '/' );
  if( slash == std::string::npos ) {
    return false;
  }
  const std::string dir = fullPath.substr( 0, slash );
  if( dir.size() < std::strlen( kModsRoot ) || dir.compare( 0, std::strlen( kModsRoot ), kModsRoot ) != 0 ) {
    return false;
  }
  if( dir.size() == std::strlen( kModsRoot ) ) {
    return true;
  }

  std::string current = kModsRoot;
  size_t pos = std::strlen( kModsRoot );
  if( pos < dir.size() && dir[pos] == '/' ) {
    ++pos;
  }
  while( pos <= dir.size() ) {
    size_t next = dir.find( '/', pos );
    if( next == std::string::npos ) {
      next = dir.size();
    }
    if( next > pos ) {
      current += '/';
      current += dir.substr( pos, next - pos );
      if( !ensureDirectory( current ) ) {
        return false;
      }
    }
    if( next == dir.size() ) {
      break;
    }
    pos = next + 1;
  }
  return true;
}

bool extractZipArchiveToMods( const std::string& zipPath ) {
  unzFile zip = unzOpen64( zipPath.c_str() );
  if( zip == nullptr ) {
    return false;
  }

  bool ok = true;
  if( unzGoToFirstFile( zip ) != UNZ_OK ) {
    ok = false;
  }

  std::vector<char> buffer( kZipReadBufferSize );
  while( ok ) {
    unz_file_info64 info{};
    char rawName[1024] = {};
    int rc = unzGetCurrentFileInfo64( zip, &info, rawName, sizeof( rawName ) - 1, nullptr, 0, nullptr, 0 );
    if( rc != UNZ_OK ) {
      ok = false;
      break;
    }
    if( info.size_filename >= sizeof( rawName ) ) {
      ok = false;
      break;
    }

    std::string relativePath = sanitizeArchivePath( rawName );
    if( relativePath.empty() ) {
      ok = false;
      break;
    }

    const bool isDir = relativePath.back() == '/' || rawName[std::strlen( rawName ) - 1] == '/';
    const std::string outPath = std::string( kModsRoot ) + "/" + relativePath;
    if( isDir ) {
      if( !ensureParentDirectories( outPath + "/.dir" ) ) {
        ok = false;
        break;
      }
      if( !ensureDirectory( outPath ) ) {
        ok = false;
        break;
      }
    }
    else {
      if( !ensureParentDirectories( outPath ) ) {
        ok = false;
        break;
      }
      if( outPath == zipPath ) {
        ok = false;
        break;
      }
      if( unzOpenCurrentFile( zip ) != UNZ_OK ) {
        ok = false;
        break;
      }

      FILE* out = std::fopen( outPath.c_str(), "wb" );
      if( out == nullptr ) {
        unzCloseCurrentFile( zip );
        ok = false;
        break;
      }

      while( true ) {
        const int readBytes = unzReadCurrentFile( zip, buffer.data(), static_cast<unsigned>( buffer.size() ) );
        if( readBytes < 0 ) {
          ok = false;
          break;
        }
        if( readBytes == 0 ) {
          break;
        }
        const size_t written = std::fwrite( buffer.data(), 1, static_cast<size_t>( readBytes ), out );
        if( written != static_cast<size_t>( readBytes ) ) {
          ok = false;
          break;
        }
      }

      std::fclose( out );
      if( unzCloseCurrentFile( zip ) != UNZ_OK ) {
        ok = false;
      }
      if( !ok ) {
        std::remove( outPath.c_str() );
        break;
      }
    }

    rc = unzGoToNextFile( zip );
    if( rc == UNZ_END_OF_LIST_OF_FILE ) {
      break;
    }
    if( rc != UNZ_OK ) {
      ok = false;
      break;
    }
  }

  unzClose( zip );
  if( ok ) {
    std::remove( zipPath.c_str() );
  }
  return ok;
}

int extractTopLevelZipArchives() {
  DIR* dir = opendir( kModsRoot );
  if( dir == nullptr ) {
    return 0;
  }

  std::vector<std::string> archives;
  while( dirent* entry = readdir( dir ) ) {
    if( entry->d_name[0] == '.' ) {
      continue;
    }
    const std::string name = entry->d_name;
    if( hasSuffixIgnoreCase( name, ".zip" ) ) {
      archives.push_back( std::string( kModsRoot ) + "/" + name );
    }
  }
  closedir( dir );

  int extracted = 0;
  for( const std::string& archive : archives ) {
    setStatus( "Extracting archive: " + archive.substr( std::strlen( kModsRoot ) + 1 ) );
    if( extractZipArchiveToMods( archive ) ) {
      ++extracted;
    }
  }
  return extracted;
}

void resetToStoppedState( const std::string& statusMessage = "MTP stopped." ) {
  setMtpKeepAwake( false );
  g_starting.store( false );
  g_running.store( false );
  g_stopping.store( false );
  g_stopRequested.store( false );
  g_disconnectDetected.store( false );
  g_lastUsbState.store( static_cast<int>( UsbState_Detached ) );
  g_waitingForHostSinceMs.store( 0 );
  g_pendingStartDelayMs.store( 0 );
  if( !g_restartRequested.load() ) {
    g_waitingForHostRestartCount.store( 0 );
  }
  g_lastTransitionMs.store( nowMs() );
  setStatus( statusMessage );
}

void mtpWorker() {
  g_workerActive.store( true );
  const long long startDelayMs = g_pendingStartDelayMs.exchange( 0 );
  if( startDelayMs > 0 ) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( startDelayMs );
    while( std::chrono::steady_clock::now() < deadline ) {
      if( g_appShuttingDown.load() || g_stopRequested.load() || g_stopping.load() ) {
        resetToStoppedState( "MTP stopped." );
        g_workerActive.store( false );
        return;
      }
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
  }

  if( g_appShuttingDown.load() || g_stopRequested.load() || g_stopping.load() ) {
    resetToStoppedState( "MTP stopped." );
    g_workerActive.store( false );
    return;
  }

  struct usb_device_descriptor deviceDescriptor = {
      .bLength = USB_DT_DEVICE_SIZE,
      .bDescriptorType = USB_DT_DEVICE,
      .bcdUSB = 0x0110,
      .bDeviceClass = 0x00,
      .bDeviceSubClass = 0x00,
      .bDeviceProtocol = 0x00,
      .bMaxPacketSize0 = 0x40,
      .idVendor = 0x057e,
      .idProduct = 0x4000,
      .bcdDevice = 0x0100,
      .bNumConfigurations = 0x01,
  };

  UsbInterfaceDesc infos[1];
  g_mtpInterface = new USBMtpInterface( 0, &infos[0] );

  setMtpKeepAwake( true );

  Result rc = usbInitialize( &deviceDescriptor, 1, infos );
  if( R_FAILED( rc ) ) {
    setStatus( "MTP failed to start (usb init error)." );
    setMtpKeepAwake( false );
    g_starting.store( false );
    g_running.store( false );
    g_stopping.store( false );
    g_stopRequested.store( false );
    g_disconnectDetected.store( false );
    g_workerActive.store( false );
    delete g_mtpInterface;
    g_mtpInterface = nullptr;
    return;
  }
  g_starting.store( false );
  g_usbInitialized.store( true );

  android::MtpStorage* storage = new android::MtpStorage(
      MTP_STORAGE_REMOVABLE_RAM,
      "sdmc:/mods/",
      "mods",
      1024U * 1024U, // 1 MiB reserved
      false,
      0xffffffffULL );

  ConfigHandler config;
  android::MtpDatabase* database = new android::SwitchMtpDatabase( config.getConfig().showDebugMtpFiles );
  database->addStoragePath( "sdmc:/mods/", "mods", MTP_STORAGE_REMOVABLE_RAM, true );

  android::MtpServer* server = new android::MtpServer( g_mtpInterface, database, false, 0, 0, 0 );
  server->addStorage( storage );
  {
    std::lock_guard<std::mutex> lk( g_serverMutex );
    g_server = server;
    g_storage = storage;
    g_database = database;
  }

  // Important: if user requested stop (or we are stopping), never overwrite the UI
  // with "waiting" again — it creates confusing stuck statuses after fast stop/unplug.
  if( g_stopRequested.load() || g_stopping.load() ) {
    server->stop();
    setStatus( "Stopping MTP..." );
  }
  else {
    setStatus( "MTP waiting for USB host..." );
    server->run();
  }

  {
    std::lock_guard<std::mutex> lk( g_serverMutex );
    if( g_server == server ) {
      g_server = nullptr;
      g_storage = nullptr;
      g_database = nullptr;
    }
  }
  delete server;
  delete storage;
  delete database;
  delete g_mtpInterface;
  g_mtpInterface = nullptr;
  if( g_usbInitialized.exchange( false ) ) {
    usbExit();
  }

  if( !g_appShuttingDown.load() && !g_restartRequested.load() ) {
    const int extractedArchives = extractTopLevelZipArchives();
    if( extractedArchives > 0 ) {
      resetToStoppedState( "MTP stopped. Zip archive extracted." );
    }
    else {
      resetToStoppedState( "MTP stopped." );
    }
  }
  else {
    resetToStoppedState( "MTP stopped." );
  }
  g_workerActive.store( false );
}

} // namespace

void ModsMtpServer::start() {
  std::lock_guard<std::mutex> lock( g_mutex );
  if( g_appShuttingDown.load() ) {
    return;
  }
  if( g_stopping.load() ) {
    if( g_disconnectDetected.load() && !g_workerActive.load() && !g_running.load() && !g_starting.load() ) {
      resetToStoppedState( "MTP stopped." );
    }
    else {
      setStatus( g_disconnectDetected.load() ? "MTP stopped." : "MTP is still stopping..." );
      return;
    }
  }
  if( g_thread.joinable() && !g_workerActive.load() ) {
    g_thread.join();
  }
  if( g_workerActive.load() || g_running.load() || g_starting.load() || g_stopping.load() ) {
    if( g_disconnectDetected.load() ) {
      setStatus( "Waiting for previous MTP session to stop..." );
    }
    return;
  }
  const bool autoRestart = g_restartRequested.load();
  const long long startDelayMs = std::max( bootGuardRemainingMs(), transitionCooldownRemainingMs() );
  setStatus( autoRestart ? "Restarting MTP..." : "Starting MTP..." );
  g_stopping.store( false );
  g_stopRequested.store( false );
  g_disconnectDetected.store( false );
  g_restartRequested.store( false );
  g_waitingForHostSinceMs.store( 0 );
  g_pendingStartDelayMs.store( startDelayMs );
  if( !autoRestart ) {
    g_waitingForHostRestartCount.store( 0 );
  }
  g_starting.store( true );
  g_running.store( true );
  g_lastTransitionMs.store( nowMs() );
  g_thread = std::thread( [] { mtpWorker(); } );
}

void ModsMtpServer::stop() {
  std::lock_guard<std::mutex> lock( g_mutex );
  g_restartRequested.store( false );
  g_waitingForHostSinceMs.store( 0 );
  g_pendingStartDelayMs.store( 0 );
  g_waitingForHostRestartCount.store( 0 );
  if( ( !g_running.load() && !g_starting.load() && !g_workerActive.load() ) || g_stopping.load() ) {
    return;
  }

  if( g_starting.load() && !g_usbInitialized.load() ) {
    // Start is in progress: request deferred stop and let worker tear down safely.
    requestStop_NoLock( "Stopping MTP..." );
    return;
  }
  // Always request stop; worker thread performs the real teardown safely.
  requestStop_NoLock( "Stopping MTP..." );
  // Non-blocking stop: worker thread handles USB teardown and final state reset.
}

void ModsMtpServer::shutdownForAppExit( int timeoutMs ) {
  {
    std::lock_guard<std::mutex> lock( g_mutex );
    g_appShuttingDown.store( true );
  }

  stop();

  const auto startTs = std::chrono::steady_clock::now();
  while( g_workerActive.load() || g_running.load() || g_starting.load() || g_stopping.load() ) {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>( now - startTs ).count();
    if( elapsedMs >= timeoutMs ) {
      break;
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  }

  setMtpKeepAwake( false );

  std::lock_guard<std::mutex> lock( g_mutex );
  if( g_thread.joinable() ) {
    if( g_workerActive.load() || g_running.load() || g_starting.load() || g_stopping.load() ) {
      // Never let std::thread stay joinable at process shutdown.
      g_thread.detach();
      setStatus( "MTP shutdown pending in background..." );
    }
    else {
      g_thread.join();
    }
  }
}

bool ModsMtpServer::isRunning() {
  return g_workerActive.load() || g_running.load() || g_starting.load() || g_stopping.load();
}

std::string ModsMtpServer::getStatusLine() {
  // Self-heal stale transitional flags to avoid UI being stuck on
  // "Starting..." / "Stopping..." after fast unplug, tab switches or app lifecycle edges.
  const long long elapsedSinceTransition = nowMs() - g_lastTransitionMs.load();
  if( g_stopping.load() && hasNoActiveSession() ) {
    g_stopping.store( false );
    if( getStatus().find( "Stopping MTP" ) != std::string::npos ) {
      setStatus( "MTP stopped." );
    }
  }
  if( g_stopping.load() && elapsedSinceTransition > kMtpStoppingTextMaxMs && !g_workerActive.load() ) {
    g_stopping.store( false );
    setStatus( "MTP stopped." );
  }
  if( g_stopping.load() && elapsedSinceTransition > kMtpStopRetryMs && g_workerActive.load() ) {
    requestStop_NoLock( g_restartRequested.load() ? "Restarting MTP..." : "Stopping MTP..." );
  }
  if( g_starting.load() && elapsedSinceTransition > kMtpStartingTextMaxMs && !g_usbInitialized.load() ) {
    g_starting.store( false );
    g_running.store( false );
    setStatus( "MTP start timeout. Please try again." );
  }

  // If there is no active session, never show "waiting for USB host".
  if( hasNoActiveSession() ) {
    const std::string s = getStatus();
    if( s.find( "MTP waiting for USB host" ) != std::string::npos ) {
      setStatus( "MTP stopped." );
    }
  }

  if( g_restartRequested.load() && hasNoActiveSession() && !g_appShuttingDown.load() ) {
    ModsMtpServer::start();
    return getStatus();
  }

  if( g_starting.load() ) {
    return "Starting MTP...";
  }
  if( g_disconnectDetected.load() && ( g_stopping.load() || g_stopRequested.load() ) ) {
    setStatus( "MTP stopped." );
    return "MTP stopped.";
  }
  if( g_stopping.load() || g_stopRequested.load() ) {
    return g_restartRequested.load() ? "Restarting MTP..." : "Stopping MTP...";
  }
  if( g_running.load() || g_workerActive.load() ) {
    if( g_stopRequested.load() ) {
      return g_restartRequested.load() ? "Restarting MTP..." : "Stopping MTP...";
    }
    UsbState state = UsbState_Detached;
    if( g_usbInitialized.load() && R_SUCCEEDED( usbDsGetState( &state ) ) ) {
      const UsbState previousState = static_cast<UsbState>( g_lastUsbState.exchange( static_cast<int>( state ) ) );

      // If USB was connected and is now unplugged/not configured, request a clean stop.
      if( state != UsbState_Configured && previousState == UsbState_Configured ) {
        g_disconnectDetected.store( true );
        requestStop_NoLock( "MTP stopped." );
        return "MTP stopped.";
      }

      if( state == UsbState_Configured ) {
        g_waitingForHostSinceMs.store( 0 );
        g_waitingForHostRestartCount.store( 0 );
        return "MTP running.";
      }
      if( g_stopRequested.load() ) {
        return g_restartRequested.load() ? "Restarting MTP..." : "Stopping MTP...";
      }
      if( shouldRestartWhileWaitingForHost( state ) ) {
        requestStop_NoLock( "Restarting MTP..." );
        return "Restarting MTP...";
      }
      return "MTP waiting for USB host...";
    }
    if( g_stopRequested.load() ) {
      return g_restartRequested.load() ? "Restarting MTP..." : "Stopping MTP...";
    }
    return "MTP waiting for USB host...";
  }
  const std::string status = getStatus();
  if( status.find("Stopping MTP") != std::string::npos && hasNoActiveSession() ) {
    return "MTP stopped.";
  }
  return status;
}
