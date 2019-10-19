#include <switch.h>

#include <browser.h>
#include <toolbox.h>
#include <mod_manager.h>
#include <parameters_handler.h>

#include <functional>
#include <iostream>
#include <sstream>

browser __br__;
mod_manager __mm__;
parameters_handler __ph__;

void print_menu();
void check_mod_status(browser &br, mod_manager &mm);

int main(int argc, char **argv)
{
  consoleInit(nullptr);

  int max_depth = 1;

  __ph__.initialize();

  __mm__.initialize();
  __mm__.set_install_mods_base_folder(__ph__.get_parameter("install-mods-base-folder"));

  __br__.set_only_show_folders(true);
  __br__.set_base_folder(__ph__.get_parameter("stored-mods-base-folder"));
  __br__.set_max_depth(max_depth);
  __br__.initialize();
  print_menu();

  // Main loop
  while(appletMainLoop())
  {
    //Scan all the inputs. This should be done once for each frame
    hidScanInput();

    //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

    if (kDown & KEY_PLUS or kDown & KEY_MINUS) {
      break; // break in order to return to hbmenu
    } else if(kDown & KEY_DOWN){
      __br__.get_browser_selector().increment_cursor_position();
    } else if(kDown & KEY_UP){
      __br__.get_browser_selector().decrement_cursor_position();
    } else if(kDown & KEY_A){
      if(__br__.go_to_selected_directory()){
        check_mod_status(__br__, __mm__);
      } else if(__br__.get_current_depth() == __br__.get_max_depth()) {
        // make mod action (ACTIVATE OR DEACTIVATE)
        __mm__.apply_mod(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selected_string());

        toolbox::print_left("Checking installed mod...", toolbox::red_bg);
        __br__.get_browser_selector().set_tag(
            __br__.get_browser_selector().get_selected_entry(),
            __mm__.get_mod_status(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selected_string())
        );
      }
    } else if(kDown & KEY_B){
      if(__br__.get_current_depth() == 0){
        break;
      }
      if(__br__.go_back()){
        // ok
      } else {
        std::cout << "Can't go back." << std::endl;
      }
    } else if(kDown & KEY_X){
      if(__br__.get_current_depth() == __br__.get_max_depth()){
        __mm__.remove_mod(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selected_string());
        __br__.get_browser_selector().set_tag(
            __br__.get_browser_selector().get_selected_entry(),
            __mm__.get_mod_status(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selected_string())
        );
      }
    } else if(kDown & KEY_ZL){
      if(__br__.get_current_depth() == __br__.get_max_depth()){
        __mm__.display_mod_files_status(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selected_string());
      }
    } else if(kDown & KEY_ZR){

      std::vector<std::string> options;
      options.emplace_back("Yes");
      options.emplace_back("No");
      std::string answer = toolbox::ask_question("Do you want to disable all mods ?", options);
      if(answer == "Yes") {
        if(__br__.get_current_depth() == __br__.get_max_depth()){
          for(int i_mod = 0 ; i_mod < int(__br__.get_browser_selector().get_selection_list().size()) ; i_mod++){
            __mm__.remove_mod(__br__.get_current_directory() + "/" + __br__.get_browser_selector().get_selection_list()[i_mod]);
          }
          check_mod_status(__br__, __mm__);
        }
      }
    } else if(kDown & KEY_L){
      if(__br__.get_current_depth() == __br__.get_max_depth()){
        __br__.get_browser_selector().previous_page();
      }
    } else if(kDown & KEY_R){
      if(__br__.get_current_depth() == __br__.get_max_depth()){
        __br__.get_browser_selector().next_page();
      }
    } else if(kDown & KEY_Y){
      if(__br__.get_current_depth() == 0){
        __ph__.increment_selected_preset_id();
        __mm__.set_install_mods_base_folder(__ph__.get_parameter("install-mods-base-folder"));
      }
    }

    if(kDown != 0){
      print_menu();
    }
  }

  consoleExit(nullptr);
  return 0;
}

void print_menu() {
  consoleClear();
  toolbox::print_right("SimpleModManager v"+toolbox::get_app_version());
  __br__.print_ls();
  std::cout << "Page (" << __br__.get_browser_selector().get_current_page()+1 << "/" << __br__.get_browser_selector().get_nb_pages() << ")" << std::endl;
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  toolbox::print_left("Configuration preset : " + __ph__.get_selected_preset_name());
  toolbox::print_left("install-mods-base-folder = " + __mm__.get_install_mods_base_folder());
  std::cout << toolbox::repeat_string("*",toolbox::get_terminal_width());
  if(__br__.get_current_depth() == __br__.get_max_depth()){
    toolbox::print_left_right(" A : Apply mod", "X : Disable mod ");
    toolbox::print_left_right(" ZL : Mod status", "ZR : Disable all mods ");
  } else{
    toolbox::print_left_right(" A : Select folder", "Y : Change config preset ");
  }
  toolbox::print_left_right(" B : Go back", "+/- : Quit ");
  if(__br__.get_browser_selector().get_nb_pages() > 1) toolbox::print_left_right(" L : Previous Page", "R : Next Page ");
  consoleUpdate(nullptr);
}
void check_mod_status(browser &br, mod_manager &mm) {
  if(br.get_current_depth() != br.get_max_depth()) return;
  br.get_browser_selector().reset_tags_list();
  auto mods_list = br.get_browser_selector().get_selection_list();
  for(int i_mod = 0 ; i_mod < int(mods_list.size()) ; i_mod++){
    br.get_browser_selector().set_tag(i_mod, "Checking...");
    print_menu();
    std::stringstream ss;
    ss << "Checking ("<< i_mod+1 << "/" << mods_list.size() << ") : " << mods_list[i_mod] << "...";
    auto spaces = toolbox::repeat_string(" ",toolbox::get_terminal_width() - ss.str().size());
    std::cout << toolbox::red_bg << ss.str() << spaces << toolbox::reset_color;
    consoleUpdate(nullptr);
    std::string mod_path = br.get_current_directory() + "/" + mods_list[i_mod];
    br.get_browser_selector().set_tag(i_mod, mm.get_mod_status(mod_path));
  }
}