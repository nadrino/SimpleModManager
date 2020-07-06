//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "frame_root.h"
#include <GlobalObjects.h>
#include <tab_games.h>
#include <tab_general_settings.h>
#include <tab_about.h>
#include <game_browser/tab_test.h>

frame_root::frame_root() {

  this->setTitle("SimpleModManager");
  this->setFooterText(GlobalObjects::_version_str_);
  this->setIcon("romfs:/images/icon.jpg");

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

  this->addTab("Game Browser", new tab_games());
  this->addSeparator();
  this->addTab("Settings", new tab_general_settings());
  this->addTab("About", new tab_about());
//  this->addSeparator();
//  this->addTab("Test0", new tab_test(0));
//  this->addTab("Test1", new tab_test(1));
//  this->addTab("Example", testList);
//  this->addTab("Layers", testLayers);

}

bool frame_root::onCancel() {

  auto* lastFocus = brls::Application::getCurrentFocus();

  bool onCancel = TabFrame::onCancel();

  if(lastFocus == brls::Application::getCurrentFocus()){
    brls::Application::quit();
  }

  return onCancel;
}
