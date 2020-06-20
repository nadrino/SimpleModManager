#include <stdio.h>
#include <stdlib.h>

#include <toolbox.h>

#include <borealis.hpp>
#include <string>
#include <GlobalObjects.h>

bool __is_new_version__;
void run_gui();
void run_console();


int main(int argc, char* argv[])
{

  toolbox::enableEmbeddedSwitchFS();

  int max_depth = 1; // could be a parameter in the future
  GlobalObjects::get_mod_browser().set_only_show_folders(true);
  GlobalObjects::get_mod_browser().set_max_relative_depth(max_depth);
  GlobalObjects::get_mod_browser().initialize();

  __is_new_version__ = false;
  int last_version = std::stoi(
    toolbox::join_vector_string(
      toolbox::split_string(
        GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("last-program-version"),
        "."
      ),
      ""
    )
  );
  int this_version = std::stoi(
    toolbox::join_vector_string(
      toolbox::split_string(
        toolbox::get_app_version()
        ,"."),
      ""
    )
  );
  if(last_version != this_version){
    __is_new_version__ = true;
  }

  if(bool(std::stoi(GlobalObjects::get_mod_browser().get_parameters_handler().get_parameter("use-gui")))){
    run_gui();
  }
  else{
    run_console();
  }

  toolbox::disableEmbeddedSwitchFS();

  // Exit
  return EXIT_SUCCESS;
}


void run_gui(){

  // Init the app
  brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

  if (!brls::Application::init("SimpleModManager"))
  {
    brls::Logger::error("Unable to init Borealis application");
    exit(EXIT_FAILURE);
  }

  // Create a sample view
  brls::TabFrame* rootFrame = new brls::TabFrame();
  rootFrame->setTitle("SimpleModManager");
  rootFrame->setIcon("romfs:/icon/icon.png");

  brls::List* testList = new brls::List();

  brls::ListItem* dialogItem = new brls::ListItem("Open a dialog");
  dialogItem->getClickEvent()->subscribe([](brls::View* view) {
    brls::Dialog* dialog = new brls::Dialog("Warning: PozzNX will wipe all data on your Switch and render it inoperable, do you want to proceed?");

    brls::GenericEvent::Callback closeCallback = [dialog](brls::View* view) {
      dialog->close();
      brls::Application::notify("Running PozzNX...");
    };

    dialog->addButton("Continue", closeCallback);
    dialog->addButton("Continue", closeCallback);
    dialog->addButton("Continue", closeCallback);

    dialog->setCancelable(false);

    dialog->open();
  });

  brls::ListItem* themeItem = new brls::ListItem("TV Resolution");
  themeItem->setValue("Automatic");

  brls::SelectListItem* jankItem = new brls::SelectListItem(
    "User Interface Jank",
    { "Native", "Minimal", "Regular", "Maximum", "SX OS", "Windows Vista", "iOS 14" });

  brls::ListItem* crashItem = new brls::ListItem("Divide by 0", "Can the Switch do it?");
  crashItem->getClickEvent()->subscribe([](brls::View* view) { brls::Application::crash("The software was closed because an error occured:\nSIGABRT (signal 6)"); });

  brls::ListItem* popupItem = new brls::ListItem("Open popup");
  popupItem->getClickEvent()->subscribe([](brls::View* view) {
    brls::TabFrame* popupTabFrame = new brls::TabFrame();
    popupTabFrame->addTab("Red", new brls::Rectangle(nvgRGB(255, 0, 0)));
    popupTabFrame->addTab("Green", new brls::Rectangle(nvgRGB(0, 255, 0)));
    popupTabFrame->addTab("Blue", new brls::Rectangle(nvgRGB(0, 0, 255)));
    brls::PopupFrame::open("Popup title", BOREALIS_ASSET("icon/borealis.jpg"), popupTabFrame, "Subtitle left", "Subtitle right");
  });

  brls::SelectListItem* layerSelectItem = new brls::SelectListItem("Select Layer", { "Layer 1", "Layer 2" });

  testList->addView(dialogItem);
  testList->addView(themeItem);
  testList->addView(jankItem);
  testList->addView(crashItem);
  testList->addView(popupItem);

  brls::Label* testLabel = new brls::Label(brls::LabelStyle::REGULAR, "For more information about how to use Nintendo Switch and its features, please refer to the Nintendo Support Website on your smart device or PC.", true);
  testList->addView(testLabel);

  brls::ListItem* actionTestItem = new brls::ListItem("Custom Actions");
  actionTestItem->registerAction("Show notification", brls::Key::L, [] {
    brls::Application::notify("Custom Action triggered");
    return true;
  });
  testList->addView(actionTestItem);

  brls::LayerView* testLayers = new brls::LayerView();
  brls::List* layerList1      = new brls::List();
  brls::List* layerList2      = new brls::List();

  layerList1->addView(new brls::Header("Layer 1", false));
  layerList1->addView(new brls::ListItem("Item 1"));
  layerList1->addView(new brls::ListItem("Item 2"));
  layerList1->addView(new brls::ListItem("Item 3"));

  layerList2->addView(new brls::Header("Layer 2", false));
  layerList2->addView(new brls::ListItem("Item 1"));
  layerList2->addView(new brls::ListItem("Item 2"));
  layerList2->addView(new brls::ListItem("Item 3"));

  testLayers->addLayer(layerList1);
  testLayers->addLayer(layerList2);

  layerSelectItem->getValueSelectedEvent()->subscribe([=](size_t selection) {
    testLayers->changeLayer(selection);
  });

  testList->addView(layerSelectItem);

  rootFrame->addTab("First tab", testList);
  rootFrame->addTab("Second tab", testLayers);
  rootFrame->addSeparator();
  rootFrame->addTab("Third tab", new brls::Rectangle(nvgRGB(255, 0, 0)));
  rootFrame->addTab("Fourth tab", new brls::Rectangle(nvgRGB(0, 255, 0)));

  // Add the root view to the stack
  brls::Application::pushView(rootFrame);

  // Run the app
  while (brls::Application::mainLoop())
    ;


}

void run_console(){

  if(__is_new_version__){
    toolbox::print_left("");
    toolbox::print_left("Welcome in SimpleModManager v" + toolbox::get_app_version(), toolbox::green_bg);
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::print_left(" > The application have successfully been upgraded.");
    toolbox::print_left("");
    toolbox::print_left("");
    toolbox::ask_question("To continue, press A.", {"Ok"});
  }

  GlobalObjects::get_mod_browser().print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if(kDown & KEY_B and GlobalObjects::get_mod_browser().get_current_relative_depth() == 0){ // back
      break;
    }

    GlobalObjects::get_mod_browser().scan_inputs(kDown, kHeld);

  } // while

}
