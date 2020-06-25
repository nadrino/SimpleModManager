//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "tab_mod_browser.h"
#include <GlobalObjects.h>
#include <thread>
#include <future>
#include <utility>

tab_mod_browser::tab_mod_browser() {



  // Setup the list
  auto mod_folders_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  for (int i_folder = 0; i_folder < int(mod_folders_list.size()); i_folder++) {
    std::string selected_mod = mod_folders_list[i_folder];
    auto* item = new brls::ListItem(selected_mod, "", "");
    item->getClickEvent()->subscribe([this, item, selected_mod](View* view) {
      brls::Logger::debug("Applying mod: %s", selected_mod.c_str());
      // apply mode
      std::string dialogTitle = "Applying: " + selected_mod + "\n";

//      auto* progressDisp = new brls::ProgressDisplay(brls::ProgressDisplayFlags::PERCENTAGE);
//      auto* dialog = new brls::Dialog("test");

//      dialog->open();
//      brls::Logger::debug("opened ?");


      auto thread = std::async(std::launch::async, [](std::string selected_mod_){
        brls::Logger::debug("t1 start");
        GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(selected_mod_, true);
        brls::Logger::debug("t1 end");
      }, selected_mod);

      thread.wait();

      // Create a promise and get its future.
//      std::promise<bool> p;
//      auto future = p.get_future();
//
//      std::thread t1([&p, selected_mod](){
//        GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(selected_mod, true);
//        brls::Logger::debug("t1 done");
//        p.set_value(true); // Is done atomically.
//      });
//
//      using namespace std::chrono_literals;
//      auto status = future.wait_for(0ms);
//
//      while(status != std::future_status::ready){
//        brls::Logger::debug("not done");
//        status = future.wait_for(10ms);
//      }
//      brls::Logger::debug("WILL JOIN");
//
//      t1.join();

      //      sleep(5); // dialog not appearing
//      dialog->close();
//      brls::Application::unblockInputs(); // should be triggered in close. why it is not ?

//      GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(selected_mod, true);

//      auto* frame = new brls::AppletFrame(false, false);
//      auto* progressBar = new brls::ProgressDisplay();
//      frame->setContentView(progressBar);
//      brls::PopupFrame::open("Popup title", BOREALIS_ASSET("icon/borealis.jpg"), frame, "Subtitle left", "Subtitle right");

      item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod));
    });
    item->registerAction("Disable", brls::Key::X, [item, selected_mod]{
      GlobalObjects::get_mod_browser().get_mod_manager().remove_mod(selected_mod);
      item->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(selected_mod));
      return true;
    });
    item->registerAction("Test", brls::Key::Y, [item, selected_mod]{
//      auto* lastFocus = brls::Application::getCurrentFocus();
      auto* progressDisp = new brls::ProgressDisplay(brls::ProgressDisplayFlags::PERCENTAGE);
      auto* dialog = new brls::Dialog(progressDisp);
      brls::GenericEvent::Callback closeCallback = [dialog](brls::View* view) {
        dialog->close();
      };
//      dialog->addButton("Cancel", closeCallback);
//      dialog->setCancelable(true);

      dialog->open();
      brls::Logger::debug("opened ?");

//      sleep(5); // dialog not appearing
      dialog->close();
      brls::Application::unblockInputs(); // should be triggered in close. why it is not ?

      //TODO: refocus the last mod


      //those arent working
//      brls::Application::giveFocus(lastFocus); // not working neither
//      brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, false);
//      brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP, false);
//      lastFocus->onFocusGained();
      return true;
    });
    item->updateActionHint(brls::Key::A, "Apply");

    this->addView(item);
    _mods_list_.emplace_back(item);
  }

  tab_mod_browser::updateModsStatus();

}

void tab_mod_browser::updateModsStatus() {

  for(auto& modItem : _mods_list_){
    modItem->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(
      modItem->getLabel()
      ));
  }

}
