//
// Embedded MTP responder (USB) for direct PC transfer into sdmc:/mods.
//

#ifndef SIMPLEMODMANAGER_MODSMTPSERVER_H
#define SIMPLEMODMANAGER_MODSMTPSERVER_H

#include <string>

class ModsMtpServer {
public:
  static void start();
  static void stop();
  static void shutdownForAppExit( int timeoutMs = 2000 );
  static bool isRunning();
  static std::string getStatusLine();
};

#endif
