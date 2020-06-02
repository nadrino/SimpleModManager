#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include <toolbox.h>
#include <mod_browser.h>
#include <sstream>

mod_browser __mod_browser__;

class ThridLevelGui : public tsl::Gui {
public:

  explicit ThridLevelGui(){

  }

  tsl::elm::Element* createUI() override {

    auto *rootFrame = new tsl::elm::OverlayFrame("SimpleModManager", toolbox::get_app_version());

    // A list that can contain sub elements and handles scrolling
    auto list = new tsl::elm::List();

    // List Items
    list->addItem(new tsl::elm::CategoryHeader("Applying mod : " + __mod_browser__.get_current_directory()));


    // Add the list to the frame for it to be drawn
    rootFrame->setContent(list);

    return rootFrame;
    }

};


class SecondLevelGui : public tsl::Gui {
public:

  explicit SecondLevelGui(const std::basic_string<char>& current_sub_folder_) {
    _current_sub_folder_ = current_sub_folder_;
  }

  tsl::elm::Element* createUI() override {

    auto* rootFrame = new tsl::elm::OverlayFrame("SimpleModManager", toolbox::get_app_version());

    std::string new_path = __mod_browser__.get_current_directory() + "/" + _current_sub_folder_;
    new_path = toolbox::remove_extra_doubled_characters(new_path, "/");

    __mod_browser__.change_directory(new_path);
    __mod_browser__.get_mod_manager().set_current_mods_folder(new_path);
    __mod_browser__.get_mod_manager().set_use_cache_only_for_status_check(true);
    __mod_browser__.get_mods_preseter().read_parameter_file(new_path);

    // A list that can contain sub elements and handles scrolling
    auto* list = new tsl::elm::List();

//    std::stringstream ss_debug;
//    ss_debug << new_path << ": " << toolbox::do_path_is_folder(new_path);
//    list->addItem(new tsl::elm::CategoryHeader(ss_debug.str()));

    // List Items
    list->addItem(new tsl::elm::CategoryHeader(_current_sub_folder_));

    auto mods_list = __mod_browser__.get_selector().get_selection_list();
    for(int i_folder = 0 ; i_folder < int(mods_list.size()) ; i_folder++){
      auto *clickableListItem = new tsl::elm::ListItem(mods_list[i_folder]);
      std::string selected_mod_name = mods_list[i_folder];

      clickableListItem->setClickListener([selected_mod_name, this](u64 keys) {
        if (keys & KEY_A) {
          // apply mod...
          __mod_browser__.get_mod_manager().apply_mod(selected_mod_name, true);
          __mod_browser__.get_selector().set_tag(
            __mod_browser__.get_selector().get_entry(selected_mod_name),
            __mod_browser__.get_mod_manager().get_mod_status(selected_mod_name)
          );
          __mod_browser__.go_back();
          tsl::goBack();
          return true;
        } else if(keys & KEY_X){
          __mod_browser__.get_mod_manager().remove_mod(__mod_browser__.get_selector().get_selected_string());
          __mod_browser__.get_selector().set_tag(
            __mod_browser__.get_selector().get_selected_entry(),
            __mod_browser__.get_mod_manager().get_mod_status(__mod_browser__.get_selector().get_selected_string())
          );
//          __mod_browser__.go_back();
//          tsl::goBack();
        }
        return false;
      });
      list->addItem(clickableListItem);

      double mod_fraction = __mod_browser__.get_mod_manager().get_mod_status_fraction(mods_list[i_folder]);
      if(mod_fraction == -1){
        mod_fraction = 0;
        list->addItem(new tsl::elm::CustomDrawer([mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
          renderer->drawRect(x, y+4, 400, 10, renderer->a(tsl::Color(100, 100,100,255)));
        }), 17);
      } else {
        list->addItem(new tsl::elm::CustomDrawer([mod_fraction](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
          renderer->drawRect(x, y+4, 400*mod_fraction, 10, renderer->a(tsl::Color(100, 200,100,255)));
          renderer->drawRect(x+400*mod_fraction, y+4, 400*(1-mod_fraction), 10, renderer->a(tsl::Color(100, 100,100,255)));
        }), 17);
      }

    }

    list->addItem(new tsl::elm::CategoryHeader("Mods Preset"));

    // Add the list to the frame for it to be drawn
    rootFrame->setContent(list);

    return rootFrame;
  }

  // Called once every frame to handle inputs not handled by other UI elements
  virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
    if (keysDown & KEY_B) {
      __mod_browser__.go_back();
      tsl::goBack();
      return true;
    }
    return false;   // Return true here to singal the inputs have been consumed
  }

private:

  std::string _current_sub_folder_;

};


class FirstLevelGui : public tsl::Gui {
public:

  // Called when this Gui gets loaded to create the UI
  // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
  tsl::elm::Element* createUI() override {
    // A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
    // If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
    auto frame = new tsl::elm::OverlayFrame("SimpleModManager", toolbox::get_app_version());

    // A list that can contain sub elements and handles scrolling
    auto list = new tsl::elm::List();

    // List Items
    list->addItem(new tsl::elm::CategoryHeader("Folder : " + __mod_browser__.get_current_directory()));

    auto mod_folders_list = __mod_browser__.get_selector().get_selection_list();
    for(int i_folder = 0 ; i_folder < int(mod_folders_list.size()) ; i_folder++){
      auto *clickableListItem = new tsl::elm::ListItem(mod_folders_list[i_folder]);
      std::string selected_folder = mod_folders_list[i_folder];

      clickableListItem->setClickListener([selected_folder](u64 keys) {
        if (keys & KEY_A) {
          tsl::changeTo<SecondLevelGui>(selected_folder);
          return true;
        }
        return false;
      });
      list->addItem(clickableListItem);
    }


    list->addItem(new tsl::elm::ToggleListItem("Toggle List Item", true));

    // Custom Drawer, a element that gives direct access to the renderer
    list->addItem(new tsl::elm::CategoryHeader("Custom Drawer", true));
    list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
      renderer->drawCircle(x + 40, y + 40, 20, true, renderer->a(0xF00F));
      renderer->drawCircle(x + 50, y + 50, 20, true, renderer->a(0xF0F0));
      renderer->drawRect(x + 130, y + 30, 60, 40, renderer->a(0xFF00));
      renderer->drawString("Hello :)", false, x + 250, y + 70, 20, renderer->a(0xFF0F));
      renderer->drawRect(x + 40, y + 90, 300, 10, renderer->a(0xF0FF));
    }), 100);

    // Track bars
    list->addItem(new tsl::elm::CategoryHeader("Track bars"));
    list->addItem(new tsl::elm::TrackBar("\u2600"));
    list->addItem(new tsl::elm::StepTrackBar("\uE13C", 20));
    list->addItem(new tsl::elm::NamedStepTrackBar("\uE132", { "Selection 1", "Selection 2", "Selection 3" }));

    // Add the list to the frame for it to be drawn
    frame->setContent(list);

    // Return the frame to have it become the top level element of this Gui
    return frame;
  }

  // Called once every frame to update values
  void update() override {

  }

  // Called once every frame to handle inputs not handled by other UI elements
  bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
    return false;   // Return true here to singal the inputs have been consumed
  }

};


class SimpleModManagerOverlay : public tsl::Overlay {
public:

  // libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
  void initServices() override {
    auto rc = fsInitialize();
    if (R_FAILED(rc))
      fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    fsdevMountSdmc();
    toolbox::enableEmbeddedSwitchFS();

    __mod_browser__.set_only_show_folders(true);
    __mod_browser__.set_max_relative_depth(1);
    __mod_browser__.initialize();

    tsl::hlp::ScopeGuard dirGuard([&] {
    });

  }  // Called at the start to initialize all services necessary for this Overlay
  void exitServices() override {
    toolbox::disableEmbeddedSwitchFS();
    fsdevUnmountDevice("sdmc");
  }  // Callet at the end to clean up all services previously initialized

  void onShow() override {}    // Called before overlay wants to change from invisible to visible state
  void onHide() override {}    // Called before overlay wants to change from visible to invisible state

  std::unique_ptr<tsl::Gui> loadInitialGui() override {
    return initially<FirstLevelGui>();  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
  }
};


int main(int argc, char **argv) {
  return tsl::loop<SimpleModManagerOverlay>(argc, argv);
}
