//
// Created by Adrien BLANCHET on 21/05/2020.
//

#include "application.h"

#include <pu/ui/elm/elm_Rectangle.hpp>




void application::OnLoad()
{
  // Create the layout (calling the smart constructor above)
  this->layout = main_layout::New();

  _mod_browser_.set_only_show_folders(true);
  _mod_browser_.set_max_relative_depth(1);
  _mod_browser_.initialize();

  auto items_list = _mod_browser_.get_selector().get_selection_list();
  layout->load_items(items_list);

  // Load the layout. In applications layouts are loaded, not added into a container (you don't select an added layout, just load it from this function)
  // Simply explained: loading layout = the application will render that layout in the very next frame
  this->LoadLayout(this->layout);

  // Set a function when input is caught. This input handling will be the first one to be handled (before Layout or any Elements)
  // Using a lambda function here to simplify things
  // You can use member functions via std::bind() C++ wrapper
  this->SetOnInput([&](u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos)
                   {
                     if(Down & KEY_X) // If X is pressed, start with our dialog questions!
                     {
                       int opt = this->CreateShowDialog("Question", "Do you like apples?", { "Yes!", "No...", "Cancel" }, true); // (using latest option as cancel option)
                       if((opt == -1) || (opt == -2)) // -1 and -2 are similar, but if the user cancels manually -1 is set, other types or cancel should be -2.
                       {
                         this->CreateShowDialog("Cancel", "Last question was canceled.", { "Ok" }, true); // If we will ignore the option, it doesn't matter if this is true or false
                       }
                       else
                       {
                         switch(opt)
                         {
                           case 0: // "Yes" was selected
                             this->CreateShowDialog("Answer", "Really? I like apples too!", { "Ok" }, true); // Same here ^
                             break;
                           case 1: // "No" was selected
                             this->CreateShowDialog("Answer", "Oh, bad news then...", { "OK" }, true); // And here ^
                             break;
                         }
                       }
                     }
                     else if(Down & KEY_PLUS) // If + is pressed, exit application
                     {
                       this->Close();
                     }
                     else if(_current_app_state_ == GAME_BROWSER and Down & KEY_B) // If + is pressed, exit application
                     {
                       this->Close();
                     }
                   });



}

void application::reset() {

  _mod_browser_.reset();
  _current_app_state_ = GAME_BROWSER;

}
