//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModPresets.h"

#include "FrameModBrowser.h"

#include <GlobalObjects.h>
#include <GuiModManager.h>
#include <ThumbnailPresetEditor.h>

#include <borealis.hpp>
#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabModPresets]");
});

TabModPresets::TabModPresets(FrameModBrowser* owner_) : _owner_(owner_) {  }

void TabModPresets::setTriggerUpdateItem(bool triggerUpdateItem) {
  _triggerUpdateItem_ = triggerUpdateItem;
}

void TabModPresets::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {

  if( _triggerUpdateItem_ ){
    this->updatePresetItems();
  }

  this->brls::List::draw( vg,x,y,width,height,style,ctx );
}

void TabModPresets::assignButtons(brls::ListItem *item, bool isPreset_) {

  if( item == nullptr ) return;

  if(isPreset_){
    item->getClickEvent()->subscribe([](brls::View* view){});
    item->registerAction("Apply", brls::Key::A, [&, item]{
      auto* dialog = new brls::Dialog("Do you want disable all mods and apply the preset \"" + item->getLabel() + "\" ?");

      dialog->addButton("Yes", [&, dialog, item](brls::View* view) {
        // first, close the dialog box before the apply mod thread starts
        dialog->close();

        // starts the async routine
        _owner_->getModManager().startApplyModPresetThread(item->getLabel());
      });
      dialog->addButton("No", [dialog](brls::View* view) {
        dialog->close();
      });

      dialog->setCancelable(true);
      dialog->open();
      return true;
    });
    item->registerAction("Remove", brls::Key::X, [item]{

      auto* dialog = new brls::Dialog("Do you want to delete the preset \"" + item->getLabel() + "\" ?");

      dialog->addButton("Yes", [item, dialog](brls::View* view) {
        GlobalObjects::gGameBrowser.getModPresetHandler().deleteModPreset(item->getLabel() );
        dialog->close();
//        _triggerUpdateItem_ = true;
      });
      dialog->addButton("No", [dialog](brls::View* view) { dialog->close(); });

      dialog->setCancelable(true);
      dialog->open();
      return true;

    });
    item->registerAction("Edit", brls::Key::Y, [&, item]{
      // open editor
      auto* editor = new ThumbnailPresetEditor( _owner_ );
      editor->setPresetName(item->getLabel());
      editor->initialize();

      auto* icon = _owner_->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open("Preset Editor", icon, 0x20000, editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }
      else{
        brls::PopupFrame::open("Preset Editor", editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }

      return true;
    });

    item->updateActionHint(brls::Key::A, "Apply");
    item->updateActionHint(brls::Key::X, "Remove");
    item->updateActionHint(brls::Key::Y, "Edit");
  }
  else {
    // new preset
    item->getClickEvent()->subscribe([&](brls::View* view){

      // create new preset
      auto* editor = new ThumbnailPresetEditor( _owner_ );
      editor->initialize();

      auto* icon = _owner_->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open("Preset Editor", icon, 0x20000, editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }
      else{
        brls::PopupFrame::open("Preset Editor", editor, "Please select the mods you want to install", "The mods will be applied in the same order.");
      }

      return true;
    });
    item->updateActionHint(brls::Key::A, "Create");

  }

}

void TabModPresets::updatePresetItems() {
  _triggerUpdateItem_ = false;

  this->clear();

  auto presetsList = GlobalObjects::gGameBrowser.getModPresetHandler().getPresetsList();

  LogInfo << "Adding " << presetsList.size() << " presets..." << std::endl;

  _itemList_.clear();
  _itemList_.reserve( presetsList.size() );
  for( auto& preset : presetsList ){
    LogScopeIndent;
    LogInfo << "Adding mod preset: " << preset << std::endl;

    _itemList_.emplace_back( new brls::ListItem( preset ) );

    auto modsList = GlobalObjects::gGameBrowser.getModPresetHandler().getModsList(preset );
    _itemList_.back()->setValue(std::to_string(modsList.size()) + " mods in this set" );

    this->assignButtons( _itemList_.back(), true );
    this->addView( _itemList_.back() );
  }

  LogInfo << "(re)-Creating add button" << std::endl;

  _itemNewCreatePreset_ = new brls::ListItem("\uE402 Create a new mod preset");
  this->assignButtons(_itemNewCreatePreset_, false);
  this->addView(_itemNewCreatePreset_);

  brls::Application::unblockInputs();

  if(brls::Application::getCurrentFocus() != nullptr and brls::Application::getCurrentFocus()->isCollapsed()){
    brls::Application::getCurrentFocus()->onFocusLost();
    brls::Application::giveFocus(_itemNewCreatePreset_->getDefaultFocus());
  }

  LogInfo << "Leaving update..." << std::endl;
}




