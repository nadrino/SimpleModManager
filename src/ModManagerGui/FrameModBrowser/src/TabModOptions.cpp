//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModOptions.h"
#include "FrameModBrowser.h"




#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[TabModOptions]");
});


TabModOptions::TabModOptions(FrameModBrowser* owner_) : _owner_(owner_) {  }

void TabModOptions::buildFolderInstallPresetItem() {

  _itemConfigPreset_ = new brls::ListItem(
    "\uE255 Config preset",
    "Specify from which install folder mods from this subfolder (game) will be installed.\n",
    ""
  );

  _itemConfigPreset_->setValue("Inherited from the main menu");
  if( not this->getModManager().getCurrentPresetName().empty() ){
    _itemConfigPreset_->setValue( this->getModManager().getCurrentPresetName() );
  }

  // On click : show scrolling up menu
  _itemConfigPreset_->getClickEvent()->subscribe([this](View* view) {
    LogInfo << "Opening config preset selector..." << std::endl;

    // build the choice list + preselection
    int preSelection{0};
    std::vector<std::string> menuList;
    menuList.reserve( 1 + this->getModManager().getConfig().presetList.size() );
    menuList.emplace_back( "Inherited from the main menu" );
    int iPreset{-1};
    for( auto& preset: this->getModManager().getConfig().presetList ){
      iPreset++;
      menuList.emplace_back( preset.name + " \uE090 \"" + preset.installBaseFolder + "\"" );
      if( preset.name == this->getModManager().getCurrentPresetName() ){ preSelection = 1 + iPreset; }
    }

    // function that will set the config preset from the Dropdown menu selection (int result)
    brls::ValueSelectedEvent::Callback valueCallback = [this](int result) {
      if( result == -1 ){
        LogDebug << "Not selected. Return." << std::endl;
        // auto pop view
        return;
      }

      if( result == 0 ){
        LogDebug << "Same as config selected. Deleting file..." << std::endl;
        this->getModManager().setCustomPreset("");
        _itemConfigPreset_->setValue( "Inherited from the main menu" );
      }
      else{
        LogDebug << "Selected " << result - 1 << std::endl;
        this->getModManager().setCustomPreset( this->getModManager().getConfig().presetList[result - 1].name );
        _itemConfigPreset_->setValue( this->getModManager().getCurrentPresetName() );
      }

      _owner_->getGuiModManager().checkAllMods( true );
      brls::Application::popView();
      return;
    }; // Callback sequence

    brls::Dropdown::open(
        "Please select the config preset you want for this folder",
        menuList, valueCallback,
        preSelection,
        true
    );

  });

}
void TabModOptions::buildResetModsCacheItem() {

  _itemResetModsCache_ = new brls::ListItem(
    "\uE877 Recheck all mods",
    "\tThis option resets all mods cache status and recheck if each files is properly applied.\n"
              "\tWhen files where managed without SimpleModManager, the displayed mod status can be wrong. "
              "This typically happens when you modified some mod files, or other programs override some of the applied files.",
    ""
  );

  _itemResetModsCache_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("Do you want to reset mods cache status and recheck all mod files ?");

    dialog->addButton("Yes", [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startCheckAllModsThread();
    });
    dialog->addButton("No", [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildDisableAllMods() {

  _itemDisableAllMods_ = new brls::ListItem(
    "\uE872 Disable all installed mods",
    "This option will remove all installed mods files.\n"
              "Note: to be deleted, installed mod files need to be identical to the one present in this folder.",
    ""
  );

  _itemDisableAllMods_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("Do you want to disable all installed mods ?");

    dialog->addButton("Yes", [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startRemoveAllModsThread();
    });
    dialog->addButton("No", [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildGameIdentificationItem(){

  _itemGameIdentification_ = new brls::ListItem(
    "Associated TitleID",
    "",
    "Current value: "
  );

  if( _owner_->getIcon() != nullptr ){
    _itemGameIdentification_->setThumbnail( _owner_->getIcon(), 0x20000 );
    _itemGameIdentification_->setSubLabel(
        _itemGameIdentification_->getSubLabel() + _owner_->getTitleId()
    );
  }
  else{
    _itemGameIdentification_->setSubLabel(
        _itemGameIdentification_->getSubLabel() + "Not found."
    );
  }

}

void TabModOptions::initialize() {

  this->buildFolderInstallPresetItem();
  this->buildResetModsCacheItem();
  this->buildDisableAllMods();
  this->buildGameIdentificationItem();

  // finally add to view
  this->addView(_itemResetModsCache_);
  this->addView(_itemConfigPreset_);
  this->addView(_itemDisableAllMods_);
  this->addView(_itemGameIdentification_);

}

void TabModOptions::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {
  ScrollView::draw(vg, x, y, width, height, style, ctx);
}

const ModManager &TabModOptions::getModManager() const {
  return _owner_->getGameBrowser().getModManager();
}
ModManager &TabModOptions::getModManager() {
  return _owner_->getGameBrowser().getModManager();
}




