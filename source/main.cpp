#include <switch.h>

#include <browser.h>
#include <toolbox.h>
#include <mod_manager.h>

#include <functional>
#include <iostream>
#include <sstream>

void print_menu(browser& br);
void check_mod_status(browser &br, mod_manager &mm);

int main(int argc, char **argv)
{
  consoleInit(nullptr);

  int max_depth = 1;

  auto br = browser();
  br.set_base_folder("/mods");
  br.set_max_depth(max_depth);
  br.initialize();
  print_menu(br);

  auto mm = mod_manager();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

    if (kDown & KEY_PLUS) {
      break; // break in order to return to hbmenu
    } else if(kDown & KEY_DOWN){
      br.get_browser_selector().increment_cursor_position();
    } else if(kDown & KEY_UP){
      br.get_browser_selector().decrement_cursor_position();
    } else if(kDown & KEY_A){
      if(br.go_to_selected_directory()){
        check_mod_status(br, mm);
      } else if(br.get_current_depth() == br.get_max_depth()) {
        // make mod action (ACTIVATE OR DEACTIVATE)
        mm.apply_mod(br.get_current_directory() + "/" + br.get_browser_selector().get_selected_string());
        std::stringstream ss;
        ss << "Checking installed mod...";
        std::cout << toolbox::red_bg << ss.str();
        std::cout << toolbox::repeat_string(" ", toolbox::get_terminal_width() - int(ss.str().size()));
        std::cout << toolbox::reset_color;

        br.get_browser_selector().set_tag(
            br.get_browser_selector().get_selected_entry(),
            mm.get_mod_status(br.get_current_directory() + "/" + br.get_browser_selector().get_selected_string())
        );
      }
    } else if(kDown & KEY_B){
      if(br.get_current_depth() == 0){
        break;
      }
      if(br.go_back()){
        // ok
      } else {
        std::cout << "Can't go back." << std::endl;
      }
    } else if(kDown & KEY_X){
      if(br.get_current_depth() == br.get_max_depth()){
        mm.remove_mod(br.get_current_directory() + "/" + br.get_browser_selector().get_selected_string());
        br.get_browser_selector().set_tag(
            br.get_browser_selector().get_selected_entry(),
            mm.get_mod_status(br.get_current_directory() + "/" + br.get_browser_selector().get_selected_string())
        );
      }
    } else if(kDown & KEY_Y){
      if(br.get_current_depth() == br.get_max_depth()){
        mm.display_mod_files_status(br.get_current_directory() + "/" + br.get_browser_selector().get_selected_string());
      }
    } else if(kDown & KEY_MINUS){
      if(br.get_current_depth() == br.get_max_depth()){
        for(int i_mod = 0 ; i_mod < int(br.get_browser_selector().get_selection_list().size()) ; i_mod++){
          mm.remove_mod(br.get_current_directory() + "/" + br.get_browser_selector().get_selection_list()[i_mod]);
        }
        check_mod_status(br, mm);
      }
    } else if(kDown & KEY_L){
      if(br.get_current_depth() == br.get_max_depth()){
        br.get_browser_selector().previous_page();
      }
    } else if(kDown & KEY_R){
      if(br.get_current_depth() == br.get_max_depth()){
        br.get_browser_selector().next_page();
      }
    }

    if(kDown != 0){
      print_menu(br);
    }
  }

  consoleExit(nullptr);
  return 0;
}

void print_menu(browser& br){
  consoleClear();
  toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());
  br.print_ls();
  std::cout << "Page (" << br.get_browser_selector().get_current_page()+1 << "/" << br.get_browser_selector().get_nb_pages() << ")" << std::endl;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  if(br.get_current_depth() == br.get_max_depth()){
    std::cout << " A : Apply mod" << std::endl;
    std::cout << " X : Disable mod" << std::endl;
    std::cout << " Y : Mod status" << std::endl;
    std::cout << " - : Disable all mods" << std::endl;
  } else{
    std::cout << " A : Select folder" << std::endl;
  }
  std::cout << " B : Go back" << std::endl;
  std::cout << " H : Force quit" << std::endl;
  std::cout << " L : Previous Page " << std::endl;
  std::cout << " R : Next Page" << std::endl;
  consoleUpdate(nullptr);
}
void check_mod_status(browser &br, mod_manager &mm) {
  if(br.get_current_depth() != br.get_max_depth()) return;
  br.get_browser_selector().reset_tags_list();
  auto mods_list = br.get_browser_selector().get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){
    print_menu(br);
    std::stringstream ss;
    ss << "Checking ("<< i_mod+1 << "/" << mods_list.size() << ") : " << mods_list[i_mod] << "...";
    auto spaces = toolbox::repeat_string(" ",toolbox::get_terminal_width() - ss.str().size());
    std::cout << toolbox::red_bg << ss.str() << spaces << toolbox::reset_color;
    consoleUpdate(nullptr);
    std::string mod_path = br.get_current_directory() + "/" + mods_list[i_mod];
    br.get_browser_selector().set_tag(i_mod, mm.get_mod_status(mod_path));
  }
}