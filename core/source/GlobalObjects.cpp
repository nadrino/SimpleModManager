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

    std::cout.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());
    std::cerr.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());
    std::clog.rdbuf(GlobalObjects::_cout_redirect_.rdbuf());

  }

  void threadApplyMod(void* arg) {
//    brls::Logger::debug("inside");
  GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(GlobalObjects::_str_buffer_, true);

  }

  void set_quit_now_triggered(bool value_){
    GlobalObjects::_quit_now_triggered_ = value_;
  }

  bool is_quit_now_triggered(){
    return GlobalObjects::_quit_now_triggered_;
  }

}
