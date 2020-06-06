//
// Created by Nadrino on 12/09/2019.
//

#ifndef MODAPPLIER_SELECTOR_H
#define MODAPPLIER_SELECTOR_H


#include <vector>
#include <string>
#include <switch/types.h>

class selector {

public:

  selector();
  ~selector();

  void initialize();
  void reset();

  void set_default_cursor_position(int default_cursor_position_);
  void set_cursor_position(int cursor_position_);
  void set_selection_list(std::vector<std::string> selection_list_);
  void set_cursor_marker(std::string cursor_marker_);
  void set_max_items_per_page(int max_items_per_page_);

  void set_tag(int entry_, std::string tag_);
  void set_tags_list(std::vector<std::string>& tags_list_);
  void set_description(int entry_, std::vector<std::string> description_lines_);
  void set_description_list(std::vector<std::vector<std::string>> descriptions_list_);

  int get_nb_pages();
  int get_current_page();
  int get_cursor_position();
  int get_selected_entry();
  int get_entry(std::string entry_name_);
  std::string get_tag(int entry_);
  std::vector<std::string> get_selection_list();

  void print_selector();
  void scan_inputs(u64 kDown, u64 kHeld);
  void reset_cursor_position();
  void reset_page();
  void reset_tags_list();
  void reset_description_list();
  void process_page_numbering();

  void increment_cursor_position();
  void decrement_cursor_position();
  void next_page();
  void previous_page();
  std::string get_selected_string();

private:

  int _default_cursor_position_;
  int _cursor_position_;
  int _current_page_;
  int _max_items_per_page_;
  int _nb_pages_;
  std::string _cursor_marker_;
  std::vector<std::string> _selection_list_;
  std::vector<std::string> _tags_list_;
  std::vector<std::vector<std::string>> _descriptions_list_;
  std::vector<std::vector<int>> _item_list_for_each_page_;

  u64 _previous_kHeld_;
  u64 _holding_tiks_;


};


#endif //MODAPPLIER_SELECTOR_H
