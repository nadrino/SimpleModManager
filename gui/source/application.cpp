//
// Created by Nadrino on 21/05/2020.
//

#include "application.h"

#include <pu/ui/elm/elm_Rectangle.hpp>

#include <toolbox.h>
#include <GlobalObjects.h>


void application::OnLoad()
{
  // Create the layout (calling the smart constructor above)
  this->_main_layout_ = main_layout::New();

  int max_depth = 1; // could be a parameter in the future
  GlobalObjects::get_mod_browser().set_only_show_folders(true);
  GlobalObjects::get_mod_browser().set_max_relative_depth(max_depth);
  GlobalObjects::get_mod_browser().initialize();

  auto items_list = GlobalObjects::get_mod_browser().get_selector().get_selection_list();
  _main_layout_->load_items(items_list);

  // Load the layout. In applications layouts are loaded, not added into a container (you don't select an added layout, just load it from this function)
  // Simply explained: loading layout = the application will render that layout in the very next frame
  this->LoadLayout(this->_main_layout_);

  // Set a function when input is caught. This input handling will be the first one to be handled (before Layout or any Elements)
  // Using a lambda function here to simplify things
  // You can use member functions via std::bind() C++ wrapper
  this->SetOnInput([&](u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos)
                   {
                     if(Down & KEY_A){
                       if(_current_app_state_ == GAME_BROWSER){
                         std::string new_path = GlobalObjects::get_mod_browser().get_current_directory() + "/" + _main_layout_->getMenuItems()->GetSelectedItem()->GetName().AsUTF8();
                         new_path = toolbox::remove_extra_doubled_characters(new_path, "/");
                         GlobalObjects::get_mod_browser().change_directory(new_path);
                         GlobalObjects::get_mod_browser().get_mod_manager().set_current_mods_folder(new_path);
                         GlobalObjects::get_mod_browser().get_mod_manager().set_use_cache_only_for_status_check(true);
                         GlobalObjects::get_mod_browser().get_mods_preseter().read_parameter_file(new_path);
                         _main_layout_->load_items(GlobalObjects::get_mod_browser().get_selector().get_selection_list());
                         _current_app_state_ = MOD_BROWSER;
                       }
                       else if(_current_app_state_ == MOD_BROWSER){
                         GlobalObjects::get_mod_browser().get_mod_manager().apply_mod(
                           _main_layout_->getMenuItems()->GetSelectedItem()->GetName().AsUTF8(),
                           true
                           );
                         GlobalObjects::get_mod_browser().get_selector().set_tag(
                           GlobalObjects::get_mod_browser().get_selector().get_entry(
                             _main_layout_->getMenuItems()->GetSelectedItem()->GetName().AsUTF8()
                             ),
                           GlobalObjects::get_mod_browser().get_mod_manager().get_mod_status(
                             _main_layout_->getMenuItems()->GetSelectedItem()->GetName().AsUTF8()
                             )
                         );
                       }

                     }
//                     if(Down & KEY_X) // If X is pressed, start with our dialog questions!
//                     {
//                       int opt = this->CreateShowDialog("Question", "Do you like apples?", { "Yes!", "No...", "Cancel" }, true); // (using latest option as cancel option)
//                       if((opt == -1) || (opt == -2)) // -1 and -2 are similar, but if the user cancels manually -1 is set, other types or cancel should be -2.
//                       {
//                         this->CreateShowDialog("Cancel", "Last question was canceled.", { "Ok" }, true); // If we will ignore the option, it doesn't matter if this is true or false
//                       }
//                       else
//                       {
//                         switch(opt)
//                         {
//                           case 0: // "Yes" was selected
//                             this->CreateShowDialog("Answer", "Really? I like apples too!", { "Ok" }, true); // Same here ^
//                             break;
//                           case 1: // "No" was selected
//                             this->CreateShowDialog("Answer", "Oh, bad news then...", { "OK" }, true); // And here ^
//                             break;
//                         }
//                       }
//                     }
                     else if(Down & KEY_B)
                     {
                       if(_current_app_state_ == GAME_BROWSER){
                         this->Close();
                       }
                       else if(_current_app_state_ == MOD_BROWSER){
                         GlobalObjects::get_mod_browser().go_back();
                         _main_layout_->load_items(GlobalObjects::get_mod_browser().get_selector().get_selection_list());
                         _current_app_state_ = GAME_BROWSER;
                       }
                     }
                   });



}

void application::reset() {

  GlobalObjects::get_mod_browser().reset();
  _current_app_state_ = GAME_BROWSER;

}
