//
// Created by Adrien BLANCHET on 26/06/2020.
//

#include "PopupApplyMod.h"
#include <GlobalObjects.h>
#include <TabModBrowser.h>

#include "Logger.h"

#include <utility>
#include "future"

LoggerInit([]{
  Logger::setUserHeaderStr("[popup_apply_mod]");
});

PopupApplyMod::PopupApplyMod(std::string text) : Dialog(text) {
  _contentView_ = nullptr;
  _mod_item_ = nullptr;
}

void PopupApplyMod::start_applying() {

  _apply_mod_thread_ = std::async(std::launch::async, [this](){
    GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(this->_mod_name_, true);
    if(this->_mod_item_ != nullptr){
      this->_mod_item_->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(this->_mod_name_));
    }

    this->close();
    LogDebug("close");
    brls::Application::unblockInputs();
    LogDebug("unblockInputs");
  });

}

void PopupApplyMod::set_mod_name(std::string mod_name_) {
  _mod_name_ = std::move(mod_name_);
}

void PopupApplyMod::set_mod_item(brls::ListItem* mod_item_){
  _mod_item_ = mod_item_;
}

void PopupApplyMod::set_contentView(brls::View* contentView_){
  _contentView_ = contentView_;
}


