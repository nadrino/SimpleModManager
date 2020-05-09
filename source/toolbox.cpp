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
#include <chrono>

#include <switch.h>

#include <zlib.h>

#include <version_config.h>
#include <selector.h>

namespace toolbox{

  static std::time_t __last_timestamp__;
  static double __last_displayed_value__ = -1;
  static bool __CRC_check_is_enabled__ = true;

  static bool __use_embedded_switch_fs__ = false;
  static char __fs_pathBuffer__[FS_MAX_PATH];
  static FsDir __fs_DirBuffer__;
  static FsFile __fs_FileBuffer__;
  static FsFileSystem __fs_FileSystemBuffer__;


  //! printout functions :
  void display_loading(int current_index_, int end_index_, std::string title_, std::string prefix_, std::string &color_str_, bool force_display_) {

    int percent = int(round(double(current_index_) / end_index_ * 100.));
    if(__last_displayed_value__ == -1) {
      set_last_timestamp();
    }
    if(
      __last_displayed_value__ == -1 or std::time(nullptr) - __last_timestamp__ >= 1 // every second
      or current_index_ == 0 // first call
      or force_display_ // display every calls
      or current_index_ >= end_index_-1 // last entry
      ){
      set_last_timestamp();
      __last_displayed_value__ = percent;
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

    std::cout << "PRESS A to continue or + to quit now." << std::endl;
    consoleUpdate(nullptr);

    std::chrono::high_resolution_clock::time_point clock_buffer = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = clock_buffer - clock_buffer;
    while(appletMainLoop()){
      hidScanInput();
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
      u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
      if (kDown & KEY_A or (kHeld & KEY_A and time_span.count() > 250)) {
        break; // break in order to return to hbmenu
      }
      if (kDown & KEY_PLUS) {
        consoleExit(nullptr);
        exit(EXIT_SUCCESS);
      }
      time_span += std::chrono::high_resolution_clock::now() - clock_buffer;
      clock_buffer = std::chrono::high_resolution_clock::now();
    }

  }

  std::string debug_string(std::string str_){

    if(not __use_embedded_switch_fs__){
      set_use_embedded_switch_fs(true);
    }

    std::stringstream ss;
    ss << str_ << ": ";

    char pathBuffer[FS_MAX_PATH];
    FsDir DirBuffer;
    s64 counter;
    Result rc;

    strcpy(pathBuffer, str_.c_str());
    fsFsOpenDirectory(&__fs_FileSystemBuffer__, &pathBuffer[0], FsDirOpenMode_ReadDirs, &DirBuffer);
//    fsDirGetEntryCount(&DirBuffer, &counter);
//    ss << counter+1 << "-> ";
//    counter = 0;

    FsDirectoryEntry dirEntry;

    for(s64 i_count = 0 ; i_count < counter; i_count++){
      rc = fsDirRead(&DirBuffer, &counter, 1, &dirEntry);
      if( R_FAILED(rc) ){
        continue;
      }
      ss<<dirEntry.name<<",";
    }

    fsDirClose(&DirBuffer);

    return ss.str();
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


  //! toolbox vars management functions :
  void reset_last_displayed_value(){
    __last_displayed_value__ = -1;
  }
  void set_last_timestamp(){
    __last_timestamp__ = std::time(nullptr);
  }
  void set_CRC_check_is_enabled(bool CRC_check_is_enabled_){
    __CRC_check_is_enabled__ = CRC_check_is_enabled_;
  }

  bool get_CRC_check_is_enabled(){
    return __CRC_check_is_enabled__;
  }


  //! generic tools functions
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


  //! Hardware functions :
  int get_terminal_width(){
    return consoleGetDefault()->consoleWidth;
  }
  int get_terminal_height(){
    return consoleGetDefault()->consoleHeight;
  }


  //! direct filesystem functions :
  void set_use_embedded_switch_fs(bool use_embedded_switch_fs_) { // for test purposes
    // enable
    if(use_embedded_switch_fs_){
      fsOpenSdCardFileSystem(&__fs_FileSystemBuffer__);
      __use_embedded_switch_fs__ = use_embedded_switch_fs_;
    }
      // disable
    else {
      fsFsClose(&__fs_FileSystemBuffer__);
      __use_embedded_switch_fs__ = use_embedded_switch_fs_;
    }
  }

  bool do_path_is_valid(std::string &path_){
    if(not __use_embedded_switch_fs__){
      struct stat buffer{};
      return (stat (path_.c_str(), &buffer) == 0);
    }
    else {
      if(do_path_is_folder(path_)){
        return true;
      }
      else return do_path_is_file(path_);
    }
  }
  bool do_path_is_folder(std::string &folder_path_){
    if(not __use_embedded_switch_fs__){
      struct stat info{};
      stat( folder_path_.c_str(), &info );
      return S_ISDIR(info.st_mode);
    }
    else {
      strcpy(__fs_pathBuffer__, folder_path_.c_str());
      if(R_FAILED(fsFsOpenDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsDirOpenMode_ReadDirs, &__fs_DirBuffer__))){
        fsDirClose(&__fs_DirBuffer__);
        return false;
      }
      else{
        fsDirClose(&__fs_DirBuffer__);
        return true;
      }
    }
  }
  bool do_path_is_file(std::string &file_path_) {
    if(not __use_embedded_switch_fs__){
      if(not do_path_is_valid(file_path_)) return false;
      return (not do_path_is_folder(file_path_));
    }
    else{
      strcpy(__fs_pathBuffer__, file_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsOpenMode_Read, &__fs_FileBuffer__))){
        fsFileClose(&__fs_FileBuffer__);
        return false;
      }
      else{
        fsFileClose(&__fs_FileBuffer__);
        return true;
      }
    }
  }
  bool do_files_are_the_same(std::string file1_path_, std::string file2_path_) {

    if(not __use_embedded_switch_fs__){
      if(not do_path_is_file(file1_path_)) return false;
      if(not do_path_is_file(file2_path_)) return false;
      if(toolbox::get_file_size(file1_path_) != toolbox::get_file_size(file2_path_)) return false; // very fast
      if(__CRC_check_is_enabled__){
        if(toolbox::get_hash_CRC32(file1_path_) != toolbox::get_hash_CRC32(file2_path_)) return false;
      }
    }
    else {

      // opening file1
      char path_buffer_file1[FS_MAX_PATH];
      FsFile fs_file1;
      strcpy(path_buffer_file1, file1_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &path_buffer_file1[0], FsOpenMode_Read, &fs_file1))){
        fsFileClose(&fs_file1);
        return false;
      }
      // opening file2
      char path_buffer_file2[FS_MAX_PATH];
      FsFile fs_file2;
      strcpy(path_buffer_file2, file2_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &path_buffer_file2[0], FsOpenMode_Read, &fs_file2))){
        fsFileClose(&fs_file1);
        fsFileClose(&fs_file2);
        return false;
      }

      // get size of file1
      s64 size_file1 = 0;
      if(R_FAILED(fsFileGetSize(&fs_file1, &size_file1))){
        fsFileClose(&fs_file1);
        fsFileClose(&fs_file2);
        return false;
      }

      // get size of file2
      s64 size_file2 = 0;
      if(R_FAILED(fsFileGetSize(&fs_file2, &size_file2))){
        fsFileClose(&fs_file1);
        fsFileClose(&fs_file2);
        return false;
      }

      // comparing file sizes
      if(size_file1 != size_file2)
        return false;

      if(__CRC_check_is_enabled__){

        size_t copy_buffer_size = 0x10000;
        u8 data_buffer_file1[copy_buffer_size];
        u8 data_buffer_file2[copy_buffer_size];

        u64 bytes_read_counter_file1 = 0;
        u64 bytes_read_counter_file2 = 0;
        u64 read_offset = 0;

        unsigned long  last_crc1 = crc32(0L, Z_NULL, 0);
        unsigned long  last_crc2 = crc32(0L, Z_NULL, 0);
        do {

          // buffering file1
          if(R_FAILED(fsFileRead(&fs_file1, read_offset, &data_buffer_file1[0], copy_buffer_size, FsReadOption_None, &bytes_read_counter_file1))){
            fsFileClose(&fs_file1);
            fsFileClose(&fs_file2);
            return false;
          }

          // buffering file2
          if(R_FAILED(fsFileRead(&fs_file2, read_offset, &data_buffer_file2[0], copy_buffer_size, FsReadOption_None, &bytes_read_counter_file2))){
            fsFileClose(&fs_file1);
            fsFileClose(&fs_file2);
            return false;
          }

          if(bytes_read_counter_file1 != bytes_read_counter_file2){
            fsFileClose(&fs_file1);
            fsFileClose(&fs_file2);
            return false;
          }

          last_crc1 = crc32(last_crc1, data_buffer_file1, bytes_read_counter_file1);
          last_crc2 = crc32(last_crc2, data_buffer_file2, bytes_read_counter_file2);
          if(last_crc1 != last_crc2){
            fsFileClose(&fs_file1);
            fsFileClose(&fs_file2);
            return false;
          }

          // preparing next loop
          read_offset += bytes_read_counter_file1;

        }
        while(s64(read_offset) < size_file1);
      }
      fsFileClose(&fs_file1);
      fsFileClose(&fs_file2);

    }

    return true;

  }

  bool copy_file(std::string &source_file_path_, std::string &destination_file_path_){

    if(not do_path_is_file(source_file_path_)){
      return false;
    }

    if(do_path_is_file(destination_file_path_)){
      delete_file(destination_file_path_);
    }
    else{
      std::string destination_folder_path = get_folder_path_from_file_path(destination_file_path_);
      if(not do_path_is_folder(destination_folder_path)){
        mkdir_path(destination_folder_path);
      }
    }

    if(not __use_embedded_switch_fs__){

      // faster but more ram is needed
      std::ifstream source(source_file_path_, std::ios::binary);
      std::ofstream dest(destination_file_path_, std::ios::binary);

      source.seekg(0, std::ios::end);
      std::ifstream::pos_type size = source.tellg();
      source.seekg(0);

      char* buffer = new char[size];

      source.read(buffer, size);
      dest.write(buffer, size);

      delete[] buffer;
      dest.close();
      source.close();

      // (alot) slower but less memory needed
//    FILE *in, *out;
//    int c;
//    if ((in = fopen(source_file_path_.c_str(),"rb")) == NULL)
//    {
//      printf("Could not open source file: %s\n",source_file_path_.c_str());
//      return 1;
//    }
//
//    if ((out = fopen(destination_file_path_.c_str(),"wb")) == NULL)
//    {
//      printf("Could not open destination file: %s\n",destination_file_path_.c_str());
//      return 1;
//    }
//
//    while ((c = fgetc(in)) != EOF)
//      fputc(c,out);
//
//    fclose(in);
//    fclose(out);

    }
    else {

      // opening source file
      char path_buffer_source[FS_MAX_PATH];
      FsFile fs_file_source;
      strcpy(path_buffer_source, source_file_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &path_buffer_source[0], FsOpenMode_Read, &fs_file_source))){
        fsFileClose(&fs_file_source);
        return false;
      }

      // get size of source file
      s64 source_size = 0;
      if(R_FAILED(fsFileGetSize(&fs_file_source, &source_size))){
        fsFileClose(&fs_file_source);
        return false;
      }

      // create destination file
      char path_buffer_destination[FS_MAX_PATH];
      FsFile fs_file_destination;
      strcpy(path_buffer_destination, destination_file_path_.c_str());
      if(R_FAILED(fsFsCreateFile(&__fs_FileSystemBuffer__, &path_buffer_destination[0], source_size, 0))){
        fsFileClose(&fs_file_source);
        return false;
      }

      // open destination file
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &path_buffer_destination[0], FsOpenMode_Write, &fs_file_destination))){
        fsFileClose(&fs_file_destination);
        fsFileClose(&fs_file_source);
        return false;
      }

      size_t copy_buffer_size = 0x10000;
      u8 data_buffer[copy_buffer_size];

      u64 bytes_read_counter = 0;
      u64 read_offset = 0;

      do {

        // buffering source file
        if(R_FAILED(fsFileRead(&fs_file_source, read_offset, &data_buffer[0], copy_buffer_size, FsReadOption_None, &bytes_read_counter))){
          fsFileClose(&fs_file_destination);
          fsFileClose(&fs_file_source);
          return false;
        }

        // dumping data in destination file
        if(R_FAILED(fsFileWrite(&fs_file_destination, read_offset, &data_buffer[0], bytes_read_counter, FsWriteOption_Flush))){
          fsFileClose(&fs_file_destination);
          fsFileClose(&fs_file_source);
          return false;
        }

        // preparing next loop
        read_offset += bytes_read_counter;

      }
      while(s64(read_offset) < source_size);

      // cleaning up
      fsFileClose(&fs_file_destination);
      fsFileClose(&fs_file_source);

    }

    return true;

  }
  bool mv_file(std::string &source_file_path_, std::string &destination_file_path_){

    if(not do_path_is_file(source_file_path_)){
      return false;
    }

    if(do_path_is_file(destination_file_path_)){
      delete_file(destination_file_path_);
    }
    else{
      std::string destination_folder_path = get_folder_path_from_file_path(destination_file_path_);
      if(not do_path_is_folder(destination_folder_path)){
        mkdir_path(destination_folder_path);
      }
    }

    if(not __use_embedded_switch_fs__){
      std::rename(source_file_path_.c_str(), destination_file_path_.c_str());
      if(not toolbox::do_path_is_file(destination_file_path_) or toolbox::do_path_is_file(source_file_path_)){
        return false;
      }
    }
    else{
      char source_char[FS_MAX_PATH];
      char dest_char[FS_MAX_PATH];
      strcpy(source_char, source_file_path_.c_str());
      strcpy(dest_char, destination_file_path_.c_str());
      if(R_FAILED(fsFsRenameFile(&__fs_FileSystemBuffer__, &source_char[0], &dest_char[0]))){
        return false;
      }
    }

    return true;
  }
  bool delete_file(std::string file_path_){
    if(not do_path_is_file(file_path_)) return true;
    if(not __use_embedded_switch_fs__){
      std::remove(file_path_.c_str());
      return not do_path_is_file(file_path_);
    }
    else{
      char fs_path_buffer[FS_MAX_PATH];
      strcpy(fs_path_buffer, file_path_.c_str());
      if(R_FAILED(fsFsDeleteFile(&__fs_FileSystemBuffer__, &fs_path_buffer[0]))){
        return false;
      }
      return true;
    }
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
          if(not __use_embedded_switch_fs__) {
            ::mkdir(current_level.c_str(), 0777);
          }
          else{
            strcpy(__fs_pathBuffer__, current_level.c_str());
            fsFsCreateDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0]);
          }
        }
        current_level += "/"; // don't forget to append a slash
      }
      return true;

  }
  bool delete_directory(std::string folder_path_){
    if(not do_folder_is_empty(folder_path_)) return false;
    if(not __use_embedded_switch_fs__){
      rmdir(folder_path_.c_str());
    }
    else{
      strcpy(__fs_pathBuffer__, folder_path_.c_str());
      if(R_FAILED(fsFsDeleteDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0]))){
        return false;
      }
    }
    if(do_path_is_folder(folder_path_)) return false;
    return true;
  }

  long int get_file_size(std::string &file_path_) {
    if(not __use_embedded_switch_fs__){
      if(not do_path_is_file(file_path_)) return 0;
      std::ifstream testFile(file_path_.c_str(), std::ios::binary);
      const auto begin = testFile.tellg();
      testFile.seekg (0, std::ios::end);
      const auto end = testFile.tellg();
      const auto fsize = (end-begin);
      return fsize;
    }
    else{
      s64 file_size;
      strcpy(__fs_pathBuffer__, file_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsOpenMode_Read, &__fs_FileBuffer__))){
        return 0;
      }
      if(R_SUCCEEDED(fsFileGetSize(&__fs_FileBuffer__, &file_size))){
        fsFileClose(&__fs_FileBuffer__);
        return (long int)(file_size);
      }
      return 0;
    }

  }
  unsigned long get_hash_CRC32(std::string file_path_){

    if(not toolbox::do_path_is_file(file_path_))
      return 0;

    if(not __use_embedded_switch_fs__){
      std::string data = toolbox::dump_file_as_string(file_path_);
      if(data.empty())
        return 0;
      unsigned long  crc = crc32(0L, Z_NULL, 0);
      return crc32(crc, (const unsigned char*)data.c_str(), data.size());
    }
    else {

      // opening source file
      strcpy(__fs_pathBuffer__, file_path_.c_str());
      if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsOpenMode_Read, &__fs_FileBuffer__))){
        fsFileClose(&__fs_FileBuffer__);
        return false;
      }

      // get size of source file
      s64 source_size = 0;
      if(R_FAILED(fsFileGetSize(&__fs_FileBuffer__, &source_size))){
        fsFileClose(&__fs_FileBuffer__);
        return false;
      }

      size_t copy_buffer_size = 0x10000;
      u8 data_buffer[copy_buffer_size];

      u64 bytes_read_counter = 0;
      u64 read_offset = 0;

      unsigned long  last_crc = crc32(0L, Z_NULL, 0);
      do {

        // buffering source file
        if(R_FAILED(fsFileRead(&__fs_FileBuffer__, read_offset, &data_buffer[0], copy_buffer_size, FsReadOption_None, &bytes_read_counter))){
          fsFileClose(&__fs_FileBuffer__);
          return false;
        }

        unsigned long  crc = crc32_combine(last_crc, u64(data_buffer), bytes_read_counter);
        last_crc = crc;

        // preparing next loop
        read_offset += bytes_read_counter;

      }
      while(s64(read_offset) < source_size);
      fsFileClose(&__fs_FileBuffer__);
      return last_crc;

    }

  }

  std::string dump_file_as_string(std::string file_path_){
    std::string data;
    if(do_path_is_file(file_path_)){
      if(not __use_embedded_switch_fs__){
        std::ifstream input_file( file_path_.c_str(), std::ios::binary | std::ios::in );
        std::ostringstream ss;
        ss << input_file.rdbuf();
        data = ss.str();
        input_file.close();
      }
      else {
        strcpy(__fs_pathBuffer__, file_path_.c_str());
        if(R_FAILED(fsFsOpenFile(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsOpenMode_Read, &__fs_FileBuffer__))){
          fsFileClose(&__fs_FileBuffer__);
          return "";
        }
        s64 file_size = 0;
        if(R_FAILED(fsFileGetSize(&__fs_FileBuffer__, &file_size))){
          fsFileClose(&__fs_FileBuffer__);
          return "";
        }
        char *buf = (char *)malloc(file_size + 1);
        u64 file_size_u = (u64) file_size;
        u64 bytes_read;
        if(R_FAILED(fsFileRead(&__fs_FileBuffer__, 0, buf, file_size_u, FsReadOption_None, &bytes_read))){
          fsFileClose(&__fs_FileBuffer__);
          return
            "";
        }
        data = std::string(buf);
        fsFileClose(&__fs_FileBuffer__);
      }
    }
    return data;
  }
  std::vector<std::string> dump_file_as_vector_string(std::string file_path_){
    std::vector<std::string> lines;
    if(do_path_is_file(file_path_)){
    }
    return lines;
  }

  std::vector<std::string> get_list_of_entries_in_folder(std::string folder_path_) {

    std::vector<std::string> entries_list;
    if(not __use_embedded_switch_fs__){
      if(not do_path_is_folder(folder_path_)) return entries_list;
      DIR* directory;
      directory = opendir(folder_path_.c_str()); //Open current-working-directory.
      if( directory == nullptr ) {
        std::cout << "Failed to open directory : " << folder_path_ << std::endl;
        return entries_list;
      }
      else {
        struct dirent* entry;
        while ( (entry = readdir(directory)) ) {
          entries_list.emplace_back(entry->d_name);
        }
        closedir(directory);
        return entries_list;
      }
    }
    else{
      strcpy(__fs_pathBuffer__, folder_path_.c_str());
      if(R_FAILED(fsFsOpenDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsDirOpenMode_ReadDirs | FsDirOpenMode_ReadFiles, &__fs_DirBuffer__))){
        fsDirClose(&__fs_DirBuffer__);
        return entries_list;
      }
      s64 entry_count;
      if(R_FAILED(fsDirGetEntryCount(&__fs_DirBuffer__, &entry_count))){
        fsDirClose(&__fs_DirBuffer__);
        return entries_list;
      }
      size_t entry_count_size_t(entry_count);
      s64 total_entries;
      std::vector<FsDirectoryEntry> fs_directory_entries(entry_count_size_t);
      if(R_FAILED(fsDirRead(&__fs_DirBuffer__, &total_entries, entry_count_size_t, &fs_directory_entries[0]))){
        fsDirClose(&__fs_DirBuffer__);
        return entries_list;
      }
      for(u32 i_entry = 0 ; i_entry < entry_count_size_t ; i_entry++){
        std::string entry_name = fs_directory_entries[i_entry].name;
        if(entry_name == "." or entry_name == ".."){
          continue;
        }
        entries_list.emplace_back(fs_directory_entries[i_entry].name);
      }
      fsDirClose(&__fs_DirBuffer__);
      return entries_list;
    }

  }
  std::vector<std::string> get_list_of_subfolders_in_folder(std::string folder_path_) {
    std::vector<std::string> subfolders_list;
    if(not __use_embedded_switch_fs__){
      if(not do_path_is_folder(folder_path_)) return subfolders_list;
      DIR* directory;
      directory = opendir(folder_path_.c_str()); //Open current-working-directory.
      if( directory == nullptr ) {
        std::cout << "Failed to open directory : " << folder_path_ << std::endl;
        return subfolders_list;
      } else {
        struct dirent* entry;
        while ( (entry = readdir(directory)) ) {
          std::string folder_candidate = folder_path_ + "/" + std::string(entry->d_name);
          if(do_path_is_folder(folder_candidate)){
            subfolders_list.emplace_back(entry->d_name);
          }
        }
        closedir(directory);
        return subfolders_list;
      }
    }
    else{
      strcpy(__fs_pathBuffer__, folder_path_.c_str());
      if(R_FAILED(fsFsOpenDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsDirOpenMode_ReadDirs, &__fs_DirBuffer__))){
        fsDirClose(&__fs_DirBuffer__);
        return subfolders_list;
      }
      s64 entry_count;
      if(R_FAILED(fsDirGetEntryCount(&__fs_DirBuffer__, &entry_count))){
        fsDirClose(&__fs_DirBuffer__);
        return subfolders_list;
      }
      size_t entry_count_size_t(entry_count);
      s64 total_entries;
      std::vector<FsDirectoryEntry> fs_directory_entries(entry_count_size_t);
      if(R_FAILED(fsDirRead(&__fs_DirBuffer__, &total_entries, entry_count_size_t, &fs_directory_entries[0]))){
        fsDirClose(&__fs_DirBuffer__);
        return subfolders_list;
      }
      for(u32 i_entry = 0 ; i_entry < entry_count_size_t ; i_entry++){
        if(fs_directory_entries[i_entry].type != FsDirEntryType_Dir) // should not be necessary
          continue;
        std::string entry_name = fs_directory_entries[i_entry].name;
        if(entry_name == "." or entry_name == ".."){
          continue;
        }
        subfolders_list.emplace_back(fs_directory_entries[i_entry].name);
      }
      fsDirClose(&__fs_DirBuffer__);
      return subfolders_list;
    }

  }
  std::vector<std::string> get_list_of_files_in_folder(std::string folder_path_){
    std::vector<std::string> files_list;
    if(not __use_embedded_switch_fs__){
      if(not do_path_is_folder(folder_path_)) return files_list;
      DIR* directory;
      directory = opendir(folder_path_.c_str()); //Open current-working-directory.
      if( directory == nullptr ) {
        std::cout << "Failed to open directory : " << folder_path_ << std::endl;
        return files_list;
      } else {
        struct dirent* entry;
        while ( (entry = readdir(directory)) ) {
          std::string file_candidate = folder_path_ + "/" + std::string(entry->d_name);
          if(do_path_is_file(file_candidate)){
            files_list.emplace_back(entry->d_name);
          }
        }
        closedir(directory);
        return files_list;
      }
    }
    else{
      strcpy(__fs_pathBuffer__, folder_path_.c_str());
      if(R_FAILED(fsFsOpenDirectory(&__fs_FileSystemBuffer__, &__fs_pathBuffer__[0], FsDirOpenMode_ReadFiles, &__fs_DirBuffer__))){
        fsDirClose(&__fs_DirBuffer__);
        return files_list;
      }
      s64 entry_count;
      if(R_FAILED(fsDirGetEntryCount(&__fs_DirBuffer__, &entry_count))){
        fsDirClose(&__fs_DirBuffer__);
        return files_list;
      }
      size_t entry_count_size_t(entry_count);
      s64 total_entries;
      std::vector<FsDirectoryEntry> fs_directory_entries(entry_count_size_t);
      if(R_FAILED(fsDirRead(&__fs_DirBuffer__, &total_entries, entry_count_size_t, &fs_directory_entries[0]))){
        fsDirClose(&__fs_DirBuffer__);
        return files_list;
      }
      for(u32 i_entry = 0 ; i_entry < entry_count_size_t ; i_entry++){
        if(fs_directory_entries[i_entry].type != FsDirEntryType_File)
          continue;
        std::string entry_name = fs_directory_entries[i_entry].name;
        if(entry_name == "." or entry_name == ".."){
          continue;
        }
        files_list.emplace_back(fs_directory_entries[i_entry].name);
      }
      fsDirClose(&__fs_DirBuffer__);
      return files_list;
    }
  }


  //! direct filesystem (not native switch fs) functions :
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


  //! indirect filesystem (through dependencies) functions :
  bool do_folder_is_empty(std::string folder_path_){
    if(not do_path_is_folder(folder_path_)) return false;
    return get_list_of_entries_in_folder(folder_path_).empty();
  }

  std::vector<std::string> get_list_files_in_subfolders(std::string &folder_path_) {

    // WARNING : Recursive function
    std::vector<std::string> output_file_paths = get_list_of_files_in_folder(folder_path_);

    auto subfolder_names_list = get_list_of_subfolders_in_folder(folder_path_);
    for(auto &subfolder_name : subfolder_names_list){
      std::string subfolder_full_path = folder_path_;
      subfolder_full_path += "/";
      subfolder_full_path += subfolder_name;
      auto subfile_names_list = get_list_files_in_subfolders(subfolder_full_path);
      for(auto &subfile_name : subfile_names_list){
        std::string relative_subfile_path;
        relative_subfile_path += subfolder_name;
        relative_subfile_path += "/";
        relative_subfile_path += subfile_name;
        output_file_paths.emplace_back(toolbox::remove_extra_doubled_characters(relative_subfile_path, "/"));
      }
    }

    return output_file_paths;

  }


  //! External function
  std::string get_app_version(){
    std::stringstream ss;
    ss << get_version_major() << "." << get_version_minor() << "." << get_version_micro();
    return ss.str();
  }

}