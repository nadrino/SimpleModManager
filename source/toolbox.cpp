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

#include <zlib.h>

#include <version_config.h>
#include <selector.h>

namespace toolbox{

  static std::time_t last_timestamp;
  static double last_displayed_value = -1;
  static bool CRC_check_is_enabled = true;

  void reset_last_displayed_value(){
    last_displayed_value = -1;
  }
  void set_last_timestamp(){
    last_timestamp = std::time(nullptr);
  }
  void set_CRC_check_is_enabled(bool CRC_check_is_enabled_){
    CRC_check_is_enabled = CRC_check_is_enabled_;
  }
  bool get_CRC_check_is_enabled(){
    return CRC_check_is_enabled;
  }

  void display_loading(int current_index_, int end_index_, std::string title_, std::string prefix_,
                       std::string &color_str_, bool force_display_) {

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
      print_left(ss.str(), color_str_, true);
      consoleUpdate(nullptr);
    }

  }
  void print_right(std::string input_, std::string color_, bool flush_){
    int nb_space_left = get_terminal_width() - input_.size();
    if(nb_space_left <= 0){ // cut the string
      input_ = input_.substr(0, input_.size() + nb_space_left - 1);
      nb_space_left = get_terminal_width() - input_.size();
    }
    if(flush_) nb_space_left-=1;
    if(flush_) std::cout << "\r";
    std::cout << color_ << repeat_string(" ", nb_space_left) << input_ << reset_color;
    if(flush_) std::cout << "\r";
    else if(int(input_.size()) > get_terminal_width()) std::cout << std::endl;
  }
  void print_left(std::string input_, std::string color_, bool flush_){
    int nb_space_left = get_terminal_width() - input_.size();
    if(nb_space_left <= 0){ // cut the string
      input_ = input_.substr(0, input_.size() + nb_space_left - 1);
      nb_space_left = get_terminal_width() - input_.size();
    }
    if(flush_) nb_space_left-=1;
    if(flush_) std::cout << "\r";
    std::cout << color_ << input_ << repeat_string(" ", nb_space_left) << reset_color;
    if(flush_) std::cout << "\r";
    else if(int(input_.size()) > get_terminal_width()) std::cout << std::endl;
  }
  void print_left_right(std::string input_left_, std::string input_right_, std::string color_){

    int nb_space_left = get_terminal_width() - input_left_.size() - input_right_.size();
    if(nb_space_left <= 0){
      input_left_ = input_left_.substr(0, input_left_.size() + nb_space_left - 1);
      nb_space_left = get_terminal_width() - input_left_.size() - input_right_.size();
    }

    std::cout << color_ << input_left_;
    std::cout << repeat_string(" ", nb_space_left);
    std::cout << input_right_;
    std::cout << reset_color;
    if(get_terminal_width() < int(input_left_.size()) + int(input_right_.size())) std::cout << std::endl;
  }
  void make_pause(){

    std::cout << "PRESS A to continue." << std::endl;
    consoleUpdate(nullptr);

    while(appletMainLoop()){
      hidScanInput();
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
      if (kDown & KEY_A) {
        break; // break in order to return to hbmenu
      }
    }


  }

  int get_terminal_width(){
    return consoleGetDefault()->consoleWidth;
  }
  int get_terminal_height(){
    return consoleGetDefault()->consoleHeight;
  }

  bool do_string_contains_substring(std::string string_, std::string substring_){
    if(substring_.size() > string_.size()) return false;
    if(string_.find(substring_) != std::string::npos) return true;
    else return false;
  }
  bool do_string_starts_with_substring(std::string string_, std::string substring_){
    if(substring_.size() > string_.size()) return false;
    return (not string_.compare(0, substring_.size(), substring_));
  }
  bool do_string_ends_with_substring(std::string string_, std::string substring_){
    if(substring_.size() > string_.size()) return false;
    return (not string_.compare(string_.size() - substring_.size(), substring_.size(), substring_));
  }
  bool do_string_in_vector(std::string &str_, std::vector<std::string>& vector_){
    for(auto const &element : vector_){
      if(element == str_) return true;
    }
    return false;
  }
  bool do_path_is_valid(std::string &path_){
    struct stat buffer{};
    return (stat (path_.c_str(), &buffer) == 0);
  }
  bool do_path_is_folder(std::string &folder_path_){
    struct stat info{};
    stat( folder_path_.c_str(), &info );
    return (info.st_mode & S_IFDIR) != 0;
  }
  bool do_path_is_file(std::string &file_path_) {
    if(do_path_is_valid(file_path_)){
      return not do_path_is_folder(file_path_);
    } else{
      return false;
    }
  }
  bool do_files_are_the_same(std::string file1_path_, std::string file2_path_) {

    if(not do_path_is_file(file1_path_)) return false;
    if(not do_path_is_file(file2_path_)) return false;
    if(toolbox::get_file_size(file1_path_) != toolbox::get_file_size(file2_path_)) return false; // very fast
    if(CRC_check_is_enabled){
      if(toolbox::get_hash_CRC32(file1_path_) != toolbox::get_hash_CRC32(file2_path_)) return false;
    }
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

    std::string destination_folder_path = get_folder_path_from_file_path(dest_);
    if(not do_path_is_folder(destination_folder_path)){
      mkdir_path(destination_folder_path);
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

  std::string get_user_string(std::string default_str_) {

    SwkbdConfig kbd;
    Result rc=0;
    rc = swkbdCreate(&kbd, 0);

    char tmpoutstr[32] = {0};

    if (R_SUCCEEDED(rc)) {

      swkbdConfigMakePresetDefault(&kbd);
      swkbdConfigSetInitialText(&kbd, default_str_.c_str());
      rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
      if (R_SUCCEEDED(rc)) {
        printf("out str: %s\n", tmpoutstr);
      }
      swkbdClose(&kbd);

    }

    return std::string(tmpoutstr);
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
  std::string get_head_path_element_name(std::string folder_path_){
    auto elements = toolbox::split_string(folder_path_, "/");
    for(int i_element = elements.size()-1; i_element >= 0 ; i_element--){
      if(not elements[i_element].empty()) return elements[i_element];
    }
    return "";
  }
  std::string join_vector_string(std::vector<std::string> string_list_, std::string delimiter_, int begin_index_, int end_index_) {

    std::string joined_string;
    if(end_index_ == 0) end_index_ = int(string_list_.size());

    // circular permutation -> python style : tab[-1] = tab[tab.size - 1]
    if(end_index_ < 0 and int(string_list_.size()) > std::fabs(end_index_)) end_index_ = int(string_list_.size()) + end_index_;

    for(int i_list = begin_index_ ; i_list < end_index_ ; i_list++){
      if(not joined_string.empty()) joined_string += delimiter_;
      joined_string += string_list_[i_list];
    }

    return joined_string;
  }
  std::string remove_extra_doubled_characters(std::string input_str_, std::string doubled_char_){

    std::vector<std::string> substr_list = split_string(input_str_, doubled_char_);
    std::vector<std::string> cleaned_substr_list;
    for(int i_substr = 0 ; i_substr < int(substr_list.size()) ; i_substr++){
      if(not substr_list[i_substr].empty())
        cleaned_substr_list.emplace_back(substr_list[i_substr]);
    }
    std::string cleaned_input_str;
    if(do_string_starts_with_substring(input_str_, doubled_char_)) cleaned_input_str += doubled_char_;
    cleaned_input_str += join_vector_string(cleaned_substr_list, doubled_char_);
    if(do_string_ends_with_substring(input_str_, doubled_char_)) cleaned_input_str += doubled_char_;
    return cleaned_input_str;

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
  std::vector<std::string> get_list_of_subfolders_in_folder(std::string folder_path_) {
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
        std::string folder_candidate = folder_path_ + "/" + std::string(entry->d_name);
        if(do_path_is_folder(folder_candidate)){
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

      std::string entry_path = folder_path_;
      entry_path += "/";
      entry_path += entry;

      if(do_path_is_folder(entry_path)){
        auto sub_files_list = get_list_files_in_subfolders(entry_path);
        for(auto const& sub_entry : sub_files_list){
          output_file_paths.emplace_back(entry + "/" + sub_entry);
        }
      } else if(do_path_is_file(entry_path)){
        output_file_paths.emplace_back(entry);
      }
    }

    return output_file_paths;
  }

  long int get_file_size(std::string &file_path_) {
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
  int get_hash_bsdChecksumFromFilepath(std::string path_){
    if(do_path_is_file(path_)){
      FILE* f = std::fopen(path_.c_str(), "r");
      int checksum = get_hash_bsdChecksumFromFile(f);
      std::fclose(f);
      return checksum;
    } else{
      return -1;
    }
  }
  int get_hash_bsdChecksumFromFile(FILE *fp) {
    int checksum = 0;             /* The checksum mod 2^16. */

    for (int ch = getc(fp); ch != EOF; ch = getc(fp)) {
      checksum = (checksum >> 1) + ((checksum & 1) << 15);
      checksum += ch;
      checksum &= 0xffff;       /* Keep it within bounds. */
    }
    return checksum;
  }
  unsigned long get_hash_CRC32(std::string file_path_){
    if(not do_path_is_file(file_path_)) return 0;

    std::ifstream input_file( file_path_.c_str(), std::ios::binary | std::ios::in );
    std::ostringstream ss;
    ss << input_file.rdbuf();
    std::string data = ss.str();

    input_file.close();

    unsigned long  crc = crc32(0L, Z_NULL, 0);
    return crc32(crc, (const unsigned char*)data.c_str(), data.size());
  }

}