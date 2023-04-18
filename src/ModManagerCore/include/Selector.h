//
// Created by Nadrino on 12/09/2019.
//

#ifndef SMM_CORE_SELECTOR_H
#define SMM_CORE_SELECTOR_H


#include <switch/types.h>

#include <vector>
#include <string>

struct SelectorEntry{
  std::string title{};
  std::string tag{};
  std::vector<std::string> description{};
};

class Selector {

public:
  Selector() = default;

  void setCursorPosition(int cursorPosition_);

  // non native setters
  void setEntryList(const std::vector<std::string>& entryTitleList_);
  void setMaxItemsPerPage(int maxItemsPerPage_);
  void setTag(size_t entryIndex_, const std::string &tag_);
  void setTagList(const std::vector<std::string>& tagList_);
  void setDescriptionList(const std::vector<std::vector<std::string>> &descriptionList_);

  [[nodiscard]] int getNbPages() const;
  [[nodiscard]] int getCurrentPage() const;
  [[nodiscard]] int getCursorPosition() const;
  [[nodiscard]] int getSelectedEntryIndex() const;
  [[nodiscard]] int fetchEntryIndex(const std::string &entryTitle_) const;
  [[nodiscard]] const std::vector<SelectorEntry> &getEntryList() const;

  void clearTags();
  void print();
  void scanInputs(u64 kDown, u64 kHeld);
  void reset_cursor_position();
  void reset_page();
  void process_page_numbering();

  void incrementCursorPosition();
  void decrementCursorPosition();
  void next_page();
  void previous_page();
  std::string getSelectedString();

  static std::string ask_question(
      const std::string& question_, const std::vector<std::string>& answers_,
      const std::vector<std::vector<std::string>>& descriptions_={}
  );


private:

  int _nbPages_{0};
  int _currentPage_{0};
  int _cursorPosition_{0};
  int _maxItemsPerPage_{30};
  int _defaultCursorPosition_{0};
  std::string _cursorMarker_{">"};

  std::vector<SelectorEntry> _entryList_;
  std::vector<std::vector<int>> _indexListForEachPage_;

  u64 _previous_kHeld_{0};
  u64 _holding_tiks_{0};


};


#endif //SMM_CORE_SELECTOR_H
