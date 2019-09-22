//
// Created by Adrien Blanchet on 03/09/2019.
//

#ifndef SWITCHTEMPLATE_BROWSER_H
#define SWITCHTEMPLATE_BROWSER_H

#include <selector.h>

#include <string>

class browser{

public:

  browser();
  ~browser();

  void initialize();
  void reset();

  void set_base_folder(std::string base_folder_);
  void set_max_depth(int max_depth_);

  int get_current_depth();
  int get_max_depth();
  std::string get_current_directory();
  std::string get_selected_entry_name();
  selector& get_browser_selector();

  void print_ls();
  bool change_directory(std::string new_directory_);
  bool go_to_selected_directory();
  bool go_back();
  int get_path_depth(std::string& path_);


private:

  int _nb_files_;
  int _max_depth_;
  int _current_depth_;
  std::string _current_directory_;
  std::string _selected_entry_name_;
  std::string _base_folder_;
  std::vector<std::string> _folder_ls_;

  selector _browser_selector_;

};

#endif //SWITCHTEMPLATE_BROWSER_H
