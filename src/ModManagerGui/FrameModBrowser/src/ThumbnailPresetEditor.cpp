//
// Created by Adrien BLANCHET on 30/06/2020.
//

#include "ThumbnailPresetEditor.h"
#include "FrameModBrowser.h"

#include <GlobalObjects.h>

#include <ranges>

#include "GenericToolbox.Switch.h"

#include "sstream"



ThumbnailPresetEditor::ThumbnailPresetEditor(FrameModBrowser* owner_, const std::string& presetName_) : _owner_(owner_){

  // fill the item list
  for( auto& availableMod : _owner_->getGameBrowser().getModManager().getModList() ){
    auto* item = new brls::ListItem( availableMod.modName, "", "" );

    // add mod
    item->getClickEvent()->subscribe([this,item](brls::View* view){
      _bufferPreset_.modList.emplace_back( item->getLabel() );
      this->updateTags();
      return true;
    });

    // remove mod
    item->registerAction("Remove", brls::Key::X, [this,item]{

      if( _bufferPreset_.modList.empty() ){
        // nothing to remove
        return true;
      }

      // loop backward to fetch the first appearing mod to be deleted
      for( auto modIterator = _bufferPreset_.modList.rbegin(); modIterator != _bufferPreset_.modList.rend(); ++modIterator ){
        if( *modIterator == item->getLabel() ) {
          // We cannot directly erase with it because once we erase an element from a container,
          // the iterator to that element and all iterators after that element are invalidated.
          _bufferPreset_.modList.erase( std::next(modIterator).base() );
          break;
        }
      }

      this->updateTags();
      return true;
    });

    // disable + to quit
    item->registerAction("", brls::Key::PLUS, []{ return true; }, true);

    // keep the pointer of the list in order to update the tags
    _availableModItemList_.emplace_back( item );
  }

  // the list that will appear
  auto* modViewList = new brls::List();
  for( auto& availableMod : _availableModItemList_ ){ modViewList->addView( availableMod ); }
  this->setContentView( modViewList );
  this->registerAction("", brls::Key::PLUS, []{return true;}, true);


  int presetIndex{-1};
  if( not presetName_.empty() ){
    presetIndex = GenericToolbox::findElementIndex(
        presetName_, _owner_->getGameBrowser().getModPresetHandler().getPresetList(),
        [](const PresetData& p ){ return p.name; }
    );
  }

  if( presetIndex == -1 ){
    // preset not found, start from scratch
    this->autoAssignPresetName();
  }
  else{
    // copy existing preset
    _bufferPreset_ = _owner_->getGameBrowser().getModPresetHandler().getPresetList()[presetIndex];
  }

  // sidebar and save button
  this->getSidebar()->setTitle( _bufferPreset_.name );
  this->getSidebar()->getButton()->getClickEvent()->subscribe([this](brls::View* view){
    this->save();

    brls::Application::popView(brls::ViewAnimation::FADE);
    brls::Application::unblockInputs();

    return true;
  });
  this->getSidebar()->registerAction("", brls::Key::PLUS, []{return true;}, true);

  this->updateTags();
}



void ThumbnailPresetEditor::updateTags() {

  // reset tags
  for( auto & availableMod : _availableModItemList_ ){  availableMod->setValue(""); }

  // set tags
  for( size_t iEntry = 0 ; iEntry < _bufferPreset_.modList.size() ; iEntry++ ){
    // loop over selected mods

    for( auto & availableMod : _availableModItemList_ ){
      // loop over available mods

      if( availableMod->getLabel() != _bufferPreset_.modList[iEntry] ){
        // wrong mod, next
        continue;
      }

      // update the tag
      std::stringstream ss;
      if( availableMod->getValue().empty() ) ss << "#" << iEntry + 1;
      else ss << availableMod->getValue() << " & #" << iEntry + 1;
      availableMod->setValue( ss.str() );

    }
  }

  std::stringstream ss;
  ss << this->_bufferPreset_.modList.size() << " mods have been selected.";
  this->getSidebar()->setSubtitle( ss.str() );

}
void ThumbnailPresetEditor::save() {

  // is it an existing preset?
  int presetIndex{
      GenericToolbox::findElementIndex(
          _bufferPreset_.name, _owner_->getGameBrowser().getModPresetHandler().getPresetList(),
          [](const PresetData& p ){ return p.name; }
      )
  };

  // let us edit the name
  _bufferPreset_.name = GenericToolbox::Switch::UI::openKeyboardUi( _bufferPreset_.name );

  // insert into preset list
  if( presetIndex != -1 ){
    _owner_->getGameBrowser().getModPresetHandler().getPresetList()[presetIndex] = _bufferPreset_;
  }
  else{
    _owner_->getGameBrowser().getModPresetHandler().getPresetList().emplace_back( _bufferPreset_ );
  }

  // TODO: Check for conflicts
//  showConflictingFiles(_presetName_);

  // save to file
  _owner_->getGameBrowser().getModPresetHandler().writeConfigFile();
  _owner_->getGameBrowser().getModPresetHandler().readConfigFile();

  // trigger backdrop list update
  _owner_->getTabModPresets()->setTriggerUpdateItem( true );
}
void ThumbnailPresetEditor::autoAssignPresetName() {
  std::string autoName = "new-preset";
  _bufferPreset_.name = autoName;
  int count{0};
  while( GenericToolbox::doesElementIsInVector(
      _bufferPreset_.name, _owner_->getGameBrowser().getModPresetHandler().getPresetList(),
      [](const PresetData& p ){ return p.name; }
  )){
    _bufferPreset_.name = autoName + "-" + std::to_string(count);
    count++;
  }
}

void ThumbnailPresetEditor::draw(
    NVGcontext *vg, int x, int y,
    unsigned int width, unsigned int height,
    brls::Style *style, brls::FrameContext *ctx) {

  if     ( _bufferPreset_.modList.empty() ){
    // save button should be hidden if nothing is selected
    this->getSidebar()->getButton()->setState(brls::ButtonState::DISABLED);
  }
  else if( this->getSidebar()->getButton()->getState() == brls::ButtonState::DISABLED ){
    // re-enable it
    this->getSidebar()->getButton()->setState(brls::ButtonState::ENABLED);
  }

  // trigger the default draw
  this->ThumbnailFrame::draw(vg, x, y, width, height, style, ctx);
}

