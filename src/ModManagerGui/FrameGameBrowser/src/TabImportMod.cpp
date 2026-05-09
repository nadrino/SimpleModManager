//
// In-app import: USB MTP responder -> sdmc:/mods
//

#include "TabImportMod.h"

#include "ModsMtpServer.h"

#include "Logger.h"

#include <borealis.hpp>

LoggerInit( [] {
  Logger::setUserHeaderStr( "[TabImportMod]" );
} );

namespace {

const char* kInstructionsEn =
    "This screen starts an in-app MTP USB responder. Windows File Explorer can copy files and folders directly "
    "to the \"mods\" folder (sdmc:/mods) while this tab is active.\n\n"
    "1. Connect the Switch to the PC with a data USB cable.\n"
    "2. Wait until the device appears in Explorer.\n"
    "3. Open the device, then copy files/folders into the mods storage.\n\n"
    "Press A to start/stop MTP.";

} // namespace

TabImportMod::TabImportMod() {
  LogWarning << "Building Import Mod tab..." << std::endl;

  this->addView( new brls::Header( "Import mod from PC" ) );

  _bodyLabel_ = new brls::Label( brls::LabelStyle::REGULAR, kInstructionsEn, true );
  _bodyLabel_->setHorizontalAlign( NVG_ALIGN_LEFT );
  this->addView( _bodyLabel_ );

  _statusLabel_ = new brls::Label( brls::LabelStyle::REGULAR, "", true );
  _statusLabel_->setHorizontalAlign( NVG_ALIGN_LEFT );
  this->addView( _statusLabel_ );

  _actionRow_ = new brls::ListItem( "Toggle MTP responder", "Press A to start or stop USB MTP mode." );
  this->addView( _actionRow_ );

  _actionRow_->registerAction( "Toggle", brls::Key::A, [this] {
    if( ModsMtpServer::isRunning() ) {
      ModsMtpServer::stop();
    }
    else {
      ModsMtpServer::start();
    }
    refreshStatusLine();
    return true;
  } );

  refreshStatusLine();

  LogInfo << "Import Mod tab built." << std::endl;
}

brls::View* TabImportMod::getDefaultFocus() {
  return _actionRow_;
}

void TabImportMod::refreshStatusLine() {
  if( _statusLabel_ == nullptr ) {
    return;
  }
  const std::string s = ModsMtpServer::getStatusLine();
  _lastStatus_ = s;
  _statusLabel_->setText( s );
  if( _actionRow_ != nullptr ) {
    if( ModsMtpServer::isRunning() ) {
      _actionRow_->setLabel( "Stop MTP responder" );
      _actionRow_->setDescription( "Press A to stop USB MTP mode." );
    }
    else {
      _actionRow_->setLabel( "Start MTP responder" );
      _actionRow_->setDescription( "Press A to expose mods folder over USB MTP." );
    }
  }
}

void TabImportMod::draw(
    NVGcontext* vg,
    int x,
    int y,
    unsigned width,
    unsigned height,
    brls::Style* style,
    brls::FrameContext* ctx ) {
  if( ++_statusTick_ >= 15 ) {
    _statusTick_ = 0;
    refreshStatusLine();
  }
  this->brls::List::draw( vg, x, y, width, height, style, ctx );
}
