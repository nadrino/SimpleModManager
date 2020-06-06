#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include <toolbox.h>

#include <GlobalObjects.h>
#include <GameBrowserGui.h>


class SimpleModManagerOverlay : public tsl::Overlay {
public:

  // libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
  void initServices() override {

    toolbox::enableEmbeddedSwitchFS();

    GlobalObjects::get_mod_browser().set_only_show_folders(true);
    GlobalObjects::get_mod_browser().set_max_relative_depth(1);
    GlobalObjects::get_mod_browser().initialize();

    tsl::hlp::ScopeGuard dirGuard([&] {
    });

  }  // Called at the start to initialize all services necessary for this Overlay
  void exitServices() override {

    toolbox::disableEmbeddedSwitchFS();

  }  // Callet at the end to clean up all services previously initialized

  void onShow() override {}    // Called before overlay wants to change from invisible to visible state
  void onHide() override {}    // Called before overlay wants to change from visible to invisible state

  std::unique_ptr<tsl::Gui> loadInitialGui() override {
    return initially<GameBrowserGui>();  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
  }
};


int main(int argc, char **argv) {
  return tsl::loop<SimpleModManagerOverlay>(argc, argv);
}
