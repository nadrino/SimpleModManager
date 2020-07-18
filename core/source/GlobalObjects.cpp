//
// Created by Nadrino on 06/06/2020.
//

#include "GlobalObjects.h"

namespace GlobalObjects{

  static bool _quit_now_triggered_ = false;

  mod_browser &get_mod_browser() {
    return _mod_browser_;
  }

  void redirect_cout(){

    GlobalObjects::_cout_backup_ = std::cout.rdbuf();

    std::cout.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());
    std::cerr.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());
    std::clog.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());

  }

  void disable_cout_redirection(){

    if(GlobalObjects::_cout_backup_ != nullptr){
      std::cout.rdbuf(GlobalObjects::_cout_backup_);
      std::cerr.rdbuf(GlobalObjects::_cout_backup_);
      std::clog.rdbuf(GlobalObjects::_cout_backup_);
    }

  }

  void setTriggerSwitchUI(bool triggerSwitchUI_){
    GlobalObjects::_triggerSwitchUI_ = std::to_string(triggerSwitchUI_);
  }

  bool doTriggerSwitchUI(){
    return toolbox::to_bool(GlobalObjects::_triggerSwitchUI_);
  }

  void set_quit_now_triggered(bool value_){
    GlobalObjects::_quit_now_triggered_ = value_;
  }

  bool is_quit_now_triggered(){
    return GlobalObjects::_quit_now_triggered_;
  }

}
