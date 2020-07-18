//
// Created by Nadrino on 04/09/2019.
//

#ifndef SIMPLEMODMANAGER_TOOLBOX_H
#define SIMPLEMODMANAGER_TOOLBOX_H

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <switch.h>

namespace toolbox{

  static std::string empty_str;
  static std::string red_bg = "\033[1;41m";
  static std::string grey_bg = "\033[1;47m";
  static std::string blue_bg = "\033[1;44m";
  static std::string green_bg = "\033[1;42m";
  static std::string magenta_bg = "\033[1;45m";
  static std::string reset_color = "\033[0m";
  static std::vector<std::string> colors_list({
                                                red_bg,
                                                grey_bg,
                                                blue_bg,
                                                green_bg,
                                                magenta_bg,
                                                reset_color});


  //! printout/printin functions :
  void display_loading(int current_index_, int end_index_, std::string title_, std::string prefix_,
    std::string &color_str_ = empty_str, bool force_display_ = false);
  void display_second_level_loading(int current_index_, int end_index_);
  void print_right(std::string input_, std::string color_ = "", bool flush_ = false);
  void print_left(std::string input_, std::string color_ = "", bool flush_ = false);
  void print_left_right(std::string input_left_, std::string input_right_, std::string color_ = "");
  void make_pause();
  void set_buffer_string(std::string str_);

  std::string debug_string(std::string str_);
  std::string get_file_size_string(std::string& file_path_);
  std::string parse_size_unit(unsigned int size_);
  std::string get_user_string(std::string default_str_ = "");
  std::string ask_question(std::string question_, std::vector<std::string> answers_,
    std::vector<std::vector<std::string>> descriptions_=std::vector<std::vector<std::string>>());
  std::string remove_color_codes_in_string(std::string &input_string_);

  //! toolbox vars management functions :
  void reset_last_displayed_value();
  void reset_progress_map();
  void reset_str_buffer_map();

  void set_last_timestamp();
  void set_CRC_check_is_enabled(bool CRC_check_is_enabled_);
  void fill_progress_map(std::string key_, double value_);
  void fill_str_buffer_map(std::string key_, std::string value_);

  double & get_progress(std::string key_);
  std::string & get_str_buffer(std::string key_);
  bool get_CRC_check_is_enabled();


  //! generic tools functions
  bool to_bool(std::string str);
  bool do_string_contains_substring(std::string string_, std::string substring_);
  bool do_string_starts_with_substring(std::string string_, std::string substring_);
  bool do_string_ends_with_substring(std::string string_, std::string substring_);
  bool do_string_in_vector(std::string &str_, std::vector<std::string>& vector_);

  std::string get_folder_path_from_file_path(std::string file_path_);
  std::string get_filename_from_file_path(std::string file_path_);
  std::string get_head_path_element_name(std::string folder_path_);
  std::string join_vector_string(std::vector<std::string> string_list_, std::string delimiter_, int begin_index_ = 0, int end_index_ = 0);
  std::string remove_extra_doubled_characters(std::string input_str_, std::string doubled_char_);
  std::string repeat_string(std::string str_, int n_times_);
  std::string replace_substring_in_string(std::string &input_str_, std::string substr_to_look_for_, std::string substr_to_replace_);

  std::vector<std::string> split_string(std::string input_string_, std::string delimiter_);


  //! Hardware functions :
  int get_terminal_width();
  int get_terminal_height();

  unsigned long get_RAM_info(std::string component_name_);
  std::string get_RAM_info_string(std::string component_name_);


  //! direct filesystem functions :
  void setFileSystemBuffer(FsFileSystem* FileSystemBuffer_); // will enable native switch FS
  void enableEmbeddedSwitchFS();
  void disableEmbeddedSwitchFS();
  void dump_string_in_file(std::string str_, std::string& path_);

  bool do_path_is_valid(std::string &path_);
  bool do_path_is_folder(std::string &folder_path_);
  bool do_path_is_file(std::string &file_path_);
  bool do_files_are_the_same(std::string file1_path_, std::string file2_path_);
  bool copy_file(std::string &source_file_path_, std::string &destination_file_path_);
  bool mv_file(std::string &source_file_path_, std::string &destination_file_path_);
  bool delete_file(std::string file_path_);
  bool mkdir_path(std::string new_folder_path_);
  bool delete_directory(std::string folder_path_);

  long int get_file_size(std::string &file_path_);
  unsigned long get_hash_CRC32(std::string file_path_);

  std::string dump_file_as_string(std::string file_path_);
  std::string get_working_directory();

  std::vector<std::string> dump_file_as_vector_string(std::string file_path_);
  std::vector<std::string> get_list_of_entries_in_folder(std::string folder_path_);
  std::vector<std::string> get_list_of_subfolders_in_folder(std::string folder_path_);
  std::vector<std::string> get_list_of_files_in_folder(std::string folder_path_);
  std::vector<std::string> get_list_files_in_subfolders(std::string &folder_path_);
  std::string to_lower_case(std::string& input_str_);

  std::string recursive_search_for_subfolder_name_like_tid(std::string folder_path_);


  //! direct filesystem (not native switch fs) functions :
  size_t get_hash_file(std::string file_path_);
  int get_hash_bsdChecksumFromFilepath(std::string path_);
  int get_hash_bsdChecksumFromFile(FILE *fp);


  //! indirect filesystem functions :
  bool do_folder_is_empty(std::string folder_path_);

  std::vector<std::string> get_list_files_in_subfolders(std::string &folder_path_);


  //! External function
  std::string get_app_version();

  //! Templates functions : defined in toolbox_impl.h
  template <typename T, typename Compare>
  std::vector<size_t> sort_permutation(std::vector<T>& vec, Compare& compare );
  template <typename T>
  std::vector<T> apply_permutation( const std::vector<T>& vec, const std::vector<std::size_t>& p );

}

#include <toolbox_impl.h>

#endif //SIMPLEMODMANAGER_TOOLBOX_H
