//
// Created by Adrien Blanchet on 04/09/2019.
//

#ifndef MODAPPLIER_TOOLBOX_H
#define MODAPPLIER_TOOLBOX_H

#include <string>
#include <vector>

namespace toolbox{

  static std::string red_bg = "\033[1;41m";
  static std::string grey_bg = "\033[1;100m";
  static std::string blue_bg = "\033[1;44m";
  static std::string reset_color = "\033[0m";

  void reset_last_displayed_value();
  void set_last_timestamp();
  void display_loading(int current_index_ = 100, int end_index_ = 100, std::string title_ = "", std::string prefix_ = "", bool force_display_ = false);
  void print_right(std::string input_, std::string color_ = "");
  void print_left(std::string input_, std::string color_ = "");
  void print_left_right(std::string input_left_, std::string input_right_, std::string color_ = "");

  int get_terminal_width();
  int get_terminal_height();
  int bsdChecksumFromFilepath(std::string path_);
  int bsdChecksumFromFile(FILE *fp);

  bool do_string_contains_substring(std::string string_, std::string substring_);
  bool do_string_starts_with_substring(std::string string_, std::string substring_);
  bool do_path_is_folder(std::string folder_path_);
  bool do_path_is_file(std::string file_path_);
  bool do_files_are_the_same(std::string file1_path_, std::string file2_path_);
  bool do_folder_is_empty(std::string folder_path_);
  bool mkdir_path(std::string new_folder_path_);
  bool rm_file(std::string file_path_);
  bool rm_dir(std::string folder_path_);
  bool copy_file(std::string source_, std::string dest_);

  std::string get_folder_path_from_file_path(std::string file_path_);
  std::string get_filename_from_file_path(std::string file_path_);
  std::string join_vector_string(std::vector<std::string> string_list_, std::string delimiter_, int begin_index_, int end_index_);
  std::string repeat_string(std::string str_, int n_times_);
  std::string ask_question(std::string question_, std::vector<std::string> answers_);
  std::string get_app_version();

  std::vector<std::string> split_string(std::string input_string_, std::string delimiter_);
  std::vector<std::string> get_list_of_entries_in_folder(std::string folder_path_);
  std::vector<std::string> get_list_of_folders_in_folder(std::string folder_path_);
  std::vector<std::string> get_list_files_in_subfolders(std::string folder_path_);

  long int get_file_size(std::string file_path_);
  size_t get_hash_file(std::string file_path_);

}

#endif //MODAPPLIER_TOOLBOX_H
