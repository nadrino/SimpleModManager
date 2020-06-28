//
// Created by Adrien BLANCHET on 26/06/2020.
//

#ifndef SIMPLEMODMANAGER_POPUP_APPLY_MOD_H
#define SIMPLEMODMANAGER_POPUP_APPLY_MOD_H

#include <borealis.hpp>
#include <future>
#include <tab_mod_browser.h>


class popup_apply_mod : public brls::Dialog {

public:
  explicit popup_apply_mod(std::string text);

  void set_mod_name(std::string mod_name_);
  void set_mod_item(brls::ListItem* mod_item_);
  void set_contentView(brls::View* contentView_);

  void start_applying();

  std::string _mod_name_;
  std::future<void> _apply_mod_thread_;
  brls::ListItem* _mod_item_;
  brls::View* _contentView_;

};


#endif //SIMPLEMODMANAGER_POPUP_APPLY_MOD_H
