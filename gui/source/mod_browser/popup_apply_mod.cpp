//
// Created by Adrien BLANCHET on 26/06/2020.
//

#include "popup_apply_mod.h"
#include <GlobalObjects.h>
#include <tab_mod_browser.h>

#include <utility>

popup_apply_mod::popup_apply_mod(std::string text) : Dialog(text) {
  _contentView_ = nullptr;
  _mod_item_ = nullptr;
}

void popup_apply_mod::start_applying() {

  _apply_mod_thread_ = std::async(std::launch::async, [this](){
    GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(this->_mod_name_, true);
    if(this->_mod_item_ != nullptr){
      this->_mod_item_->setValue(GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(this->_mod_name_));
    }

    this->close();
    brls::Logger::debug("close");
    brls::Application::unblockInputs();
    brls::Logger::debug("unblockInputs");
  });

}

void popup_apply_mod::set_mod_name(std::string mod_name_) {
  _mod_name_ = std::move(mod_name_);
}

void popup_apply_mod::set_mod_item(brls::ListItem* mod_item_){
  _mod_item_ = mod_item_;
}

void popup_apply_mod::set_contentView(brls::View* contentView_){
  _contentView_ = contentView_;
}


