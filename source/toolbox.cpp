//
// Created by Adrien Blanchet on 04/09/2019.
//

#include <toolbox.h>

#include <cstring>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <iomanip> // stringstream
#include <sstream>
#include <filesystem> // cpp 17 functions -> does not work
#include <exception>
#include <switch.h>
#include <ctime>

#include <version_config.h>
#include <selector.h>

namespace toolbox{

  static std::time_t last_timestamp;
  static double last_displayed_value = -1;

  void reset_last_displayed_value(){
    last_displayed_value = -1;
  }
  void set_last_timestamp(){
    last_timestamp = std::time(nullptr);
  }
  void display_loading(int current_index_, int end_index_, std::string title_, std::string prefix_, bool force_display_) {

    int percent = int(round(double(current_index_) / end_index_ * 100.));
    if(last_displayed_value == -1) {
      set_last_timestamp();
    }
    if(
        last_displayed_value == -1 or std::time(nullptr) - last_timestamp >= 1 // every second
        or current_index_ == 0 // first call
        or force_display_ // display every calls
        or current_index_ >= end_index_-1 // last entry
        ){
      set_last_timestamp();
      last_displayed_value = percent;
      std::stringstream ss;
      ss << prefix_ << percent << "% / " << title_;
      std::cout << "\r" << ss.str();
      std::cout << toolbox::repeat_string(" ", toolbox::get_terminal_width() - int(ss.str().size()) - 1);
      std::cout << toolbox::reset_color << "\r";
      consoleUpdate(nullptr);
    }



  }
  void print_right(std::string input_, std::string color_){
    std::cout << color_ << repeat_string(" ", get_terminal_width() - input_.size()) << input_ << reset_color;
    if(get_terminal_width() < int(input_.size())) std::cout << std::endl;
  }
  void print_left(std::string input_, std::string color_){
    std::cout << color_ << input_ << repeat_string(" ", get_terminal_width() - input_.size()) << reset_color;
    if(get_terminal_width() < int(input_.size())) std::cout << std::endl;
  }
  void print_left_right(std::string input_left_, std::string input_right_, std::string color_){
    std::cout << color_ << input_left_;
    std::cout << repeat_string(" ", get_terminal_width() - input_left_.size() - input_right_.size());
    std::cout << input_right_;
    std::cout << reset_color;
    if(get_terminal_width() < int(input_left_.size()) + int(input_right_.size())) std::cout << std::endl;
  }

  int get_terminal_width(){
    return consoleGetDefault()->consoleWidth;
  }
  int get_terminal_height(){
    return consoleGetDefault()->consoleHeight;
  }
  int bsdChecksumFromFilepath(std::string path_){
    if(do_path_is_file(path_)){
      FILE* f = std::fopen(path_.c_str(), "r");
      int checksum = bsdChecksumFromFile(f);
      std::fclose(f);
      return checksum;
    } else{
      return -1;
    }
  }
  int bsdChecksumFromFile(FILE *fp) {
    int checksum = 0;             /* The checksum mod 2^16. */

    for (int ch = getc(fp); ch != EOF; ch = getc(fp)) {
      checksum = (checksum >> 1) + ((checksum & 1) << 15);
      checksum += ch;
      checksum &= 0xffff;       /* Keep it within bounds. */
    }
    return checksum;
  }

  bool do_string_contains_substring(std::string string_, std::string substring_){
    if(string_.find(substring_) != std::string::npos) return true;
    else return false;
  }
  bool do_string_starts_with_substring(std::string string_, std::string substring_){
    return (not string_.compare(0, substring_.size(), substring_));
  }
  bool do_path_is_folder(std::string folder_path_) {
    DIR* dir;
    dir = opendir(folder_path_.c_str());
    bool is_directory = false;
    if(dir != NULL) is_directory = true;
    closedir(dir);
    return is_directory;
  }
  bool do_path_is_file(std::string file_path_) {
    struct stat buffer;
    return (stat (file_path_.c_str(), &buffer) == 0);
  }
  bool do_files_are_the_same(std::string file1_path_, std::string file2_path_) {

    if(not do_path_is_file(file1_path_)) return false;
    if(not do_path_is_file(file2_path_)) return false;

    if(toolbox::get_file_size(file1_path_) != toolbox::get_file_size(file2_path_)) return false;
//    if(bsdChecksumFromFilepath(file1_path_) != bsdChecksumFromFilepath(file2_path_)) return false;

    return true;

  }
  bool do_folder_is_empty(std::string folder_path_){
    if(not do_path_is_folder(folder_path_)) return false;
    return get_list_of_entries_in_folder(folder_path_).size() == 0;
  }
  bool mkdir_path(std::string new_folder_path_){

    if(do_path_is_folder(new_folder_path_)) return true;

    std::string current_level;
    std::string level;
    std::stringstream ss(new_folder_path_);

    // split path using slash as a separator
    while (std::getline(ss, level, '/'))
    {
      current_level += level; // append folder to the current level
      if(current_level.empty()) current_level = "/";
      // create current level
      if(not do_path_is_folder(current_level)){
        ::mkdir(current_level.c_str(),0777);
//        int n_times = 0;
//        while(::mkdir(current_level.c_str(),0777)){
//          n_times++;
//          if(n_times > 10) break;
//        }
//        std::cout << current_level << "-> n_times = " << n_times << std::endl;
      }

      current_level += "/"; // don't forget to append a slash
    }

    return true;

  }
  bool rm_file(std::string file_path_){
    std::remove(file_path_.c_str());
    if(do_path_is_file(file_path_)) return false;
    return true;
  }
  bool rm_dir(std::string folder_path_){
    if(not do_folder_is_empty(folder_path_)) return false;
    rmdir(folder_path_.c_str());
    if(do_path_is_folder(folder_path_)) return false;
    return true;
  }
  bool copy_file(std::string source_, std::string dest_){

    if(not do_path_is_folder(get_folder_path_from_file_path(dest_))){
      mkdir_path(get_folder_path_from_file_path(dest_));
//      mkdir_path(get_folder_path_from_file_path(dest_)); // sometimes doesnt work
    }
    if(do_path_is_file(dest_)) rm_file(dest_);

    std::ifstream source(source_, std::ios::binary);
    std::ofstream dest(dest_, std::ios::binary);

    source.seekg(0, std::ios::end);
    std::ifstream::pos_type size = source.tellg();
    source.seekg(0);

    char* buffer = new char[size];

    source.read(buffer, size);
    dest.write(buffer, size);

    delete[] buffer;
    source.close();
    dest.close();

//    FILE *in, *out;
//    int c;
//    if ((in = fopen(source_.c_str(),"rb")) == NULL)
//    {
//      printf("Could not open source file: %s\n",source_.c_str());
//      return 1;
//    }
//
//    if ((out = fopen(dest_.c_str(),"wb")) == NULL)
//    {
//      printf("Could not open destination file: %s\n",dest_.c_str());
//      return 1;
//    }
//
//    while ((c = fgetc(in)) != EOF)
//      fputc(c,out);
//
//    fclose(in);
//    fclose(out);

//    if(not toolbox::do_files_are_the_same(source_, dest_)){
//      return false;
//    }
    return true;

  }

  std::string get_folder_path_from_file_path(std::string file_path_){
    std::string folder_path;
    if(file_path_[0] == '/') folder_path += "/";
    auto splitted_path = split_string(file_path_, "/");
    folder_path += join_vector_string(
        splitted_path,
        "/",
        0,
        int(splitted_path.size()) - 1
        );
    return folder_path;
  }
  std::string get_filename_from_file_path(std::string file_path_){
    auto splitted_path = split_string(file_path_, "/");
    return splitted_path.back();
  }
  std::string join_vector_string(std::vector<std::string> string_list_, std::string delimiter_, int begin_index_, int end_index_) {

    std::string joined_string;
    if(end_index_ <= 0 and int(string_list_.size()) > std::fabs(end_index_)) end_index_ = int(string_list_.size()) + end_index_;

    for(int i_list = begin_index_ ; i_list < end_index_ ; i_list++){
      if(not joined_string.empty()) joined_string += delimiter_;
      joined_string += string_list_[i_list];
    }

    return joined_string;
  }
  std::string repeat_string(std::string str_, int n_times_){
    std::string out_str;
    for(int i_times = 0 ; i_times < n_times_ ; i_times++){
      out_str += str_;
    }
    return out_str;
  }
  std::string ask_question(std::string question_, std::vector<std::string> answers_){

    consoleClear();
    std::cout << question_ << std::endl;

    auto sel = selector();
    sel.set_selection_list(answers_);
    sel.print_selector();

    while(appletMainLoop()){
      //Scan all the inputs. This should be done once for each frame
      hidScanInput();

      //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

      if(kDown & KEY_DOWN){
        sel.increment_cursor_position();
        consoleClear();
        std::cout << question_ << std::endl;
        sel.print_selector();
      } else if(kDown & KEY_UP){
        sel.decrement_cursor_position();
        consoleClear();
        std::cout << question_ << std::endl;
        sel.print_selector();
      } else if(kDown & KEY_A){
        consoleClear();
        return sel.get_selected_string();
      }
      consoleUpdate(nullptr);
    }

    return "";
  }
  std::string get_app_version(){
    std::stringstream ss;
    ss << get_version_major() << "." << get_version_minor() << "." << get_version_micro();
    return ss.str();
  }

  std::vector<std::string> split_string(std::string input_string_, std::string delimiter_){

    std::vector<std::string> output_splited_string;

    const char *src = input_string_.c_str();
    const char *next = src;

    std::string out_string_piece = "";

    while ((next = std::strstr(src, delimiter_.c_str())) != NULL) {
      out_string_piece = "";
      while (src != next){
        out_string_piece += *src++;
      }
      output_splited_string.emplace_back(out_string_piece);
      /* Skip the delimiter_ */
      src += delimiter_.size();
    }

    /* Handle the last token */
    out_string_piece = "";
    while (*src != '\0')
      out_string_piece += *src++;

    output_splited_string.emplace_back(out_string_piece);

    return output_splited_string;

  }
  std::vector<std::string> get_list_of_entries_in_folder(std::string folder_path_) {
    if(not do_path_is_folder(folder_path_)) return std::vector<std::string>();
    DIR* directory;
    directory = opendir(folder_path_.c_str()); //Open current-working-directory.
    if( directory == nullptr ) {
      std::cout << "Failed to open directory : " << folder_path_ << std::endl;
      return std::vector<std::string>();
    } else {
      std::vector<std::string> entries_list;
      struct dirent* entry;
      while ( (entry = readdir(directory)) ) {
        entries_list.emplace_back(entry->d_name);
      }
      closedir(directory);
      return entries_list;
    }
  }
  std::vector<std::string> get_list_of_folders_in_folder(std::string folder_path_) {
    if(not do_path_is_folder(folder_path_)) return std::vector<std::string>();
    DIR* directory;
    directory = opendir(folder_path_.c_str()); //Open current-working-directory.
    if( directory == nullptr ) {
      std::cout << "Failed to open directory : " << folder_path_ << std::endl;
      return std::vector<std::string>();
    } else {
      std::vector<std::string> entries_list;
      struct dirent* entry;
      while ( (entry = readdir(directory)) ) {
        if(do_path_is_folder(folder_path_ + "/" + std::string(entry->d_name))){
          entries_list.emplace_back(entry->d_name);
        }
      }
      closedir(directory);
      return entries_list;
    }
  }
  std::vector<std::string> get_list_files_in_subfolders(std::string folder_path_) {

    std::vector<std::string> output_file_paths;

    // WARNING : Recursive function
    auto entries_list = get_list_of_entries_in_folder(folder_path_);
    std::string str_buffer;
    for(auto const& entry : entries_list){
      if(do_path_is_folder(folder_path_ + "/" + entry)){
        auto sub_files_list = get_list_files_in_subfolders(folder_path_ + "/" + entry);
        for(auto const& sub_entry : sub_files_list){
          output_file_paths.emplace_back(entry + "/" + sub_entry);
        }
      } else if(do_path_is_file( folder_path_ + "/" + entry)){
        output_file_paths.emplace_back(entry);
      }
    }

    return output_file_paths;
  }

  long int get_file_size(std::string file_path_) {
    if(not do_path_is_file(file_path_)) return 0;

    std::ifstream testFile(file_path_.c_str(), std::ios::binary);
    const auto begin = testFile.tellg();
    testFile.seekg (0, std::ios::end);
    const auto end = testFile.tellg();
    const auto fsize = (end-begin);
    return fsize;
  }

  size_t get_hash_file(std::string file_path_){

    std::string data;
    std::ifstream input_file( file_path_.c_str(), std::ios::binary | std::ios::in );
    if( !input_file ) return 0;

    std::ostringstream ss;
    ss << input_file.rdbuf();
    data = ss.str();

    input_file.close();

    std::hash<std::string> hash_fn;
    return hash_fn(data);

  }



}