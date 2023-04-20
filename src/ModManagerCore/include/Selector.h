//
// Created by Nadrino on 12/09/2019.
//

#ifndef SMM_CORE_SELECTOR_H
#define SMM_CORE_SELECTOR_H


#include <switch/types.h>

#include <vector>
#include <string>
#include "map"
#include "sstream"


struct MenuLine{
  std::stringstream leftPrint{};
  std::stringstream rightPrint{};

  template<typename T> MenuLine &operator<<(const T &data){
    leftPrint << data;
    return *this;
  }
  template<typename T> MenuLine &operator>>(const T &data){
    rightPrint << data;
    return *this;
  }
};

struct MenuLineList{
  std::vector<MenuLine> lineList;

  template<typename T> MenuLineList &operator<<(const T &data){
    lineList.back().leftPrint << data;
    return *this;
  }
  template<typename T> MenuLineList &operator>>(const T &data){
    lineList.back().rightPrint << data;
    return *this;
  }
  MenuLineList &operator<<(std::ostream &(*f)(std::ostream &)){
    lineList.emplace_back();
    return *this;
  }

  void clear(){ lineList.clear(); }
};

struct SelectorEntry{
  std::string title{};
  std::string tag{};
  std::vector<std::string> description{};

  [[nodiscard]] size_t getNbPrintLines() const { return 1 + description.size(); }
};


class Selector {

public:
  Selector() = default;
  explicit Selector(const std::vector<std::string>& entryTitleList_){ this->setEntryList(entryTitleList_); }

  // non native setters
  void setEntryList(const std::vector<std::string>& entryTitleList_);
  void setTag(size_t entryIndex_, const std::string &tag_);
  void setTagList(const std::vector<std::string>& tagList_);
  void setDescriptionList(const std::vector<std::vector<std::string>> &descriptionList_);
  void clearTags();
  void clearDescriptions();

  // native getters
  size_t getCursorPosition() const;
  [[nodiscard]] const std::vector<SelectorEntry> &getEntryList() const;
  MenuLineList &getHeader();
  MenuLineList &getFooter();

  // non native getters
  const SelectorEntry& getSelectedEntry() const;
  const std::string& getSelectedEntryTitle() const;
  size_t getNbMenuLines() const;
  size_t getCursorPage() const;
  size_t getNbPages() const;
  bool isSelectedEntry(const SelectorEntry& entry_) const;

  // io
  void print() const;
  void scanInputs(u64 kDown, u64 kHeld);
  void clearMenu();

  // cursor moving
  void moveCursorPosition(long cursorPosition_);
  void jumpToPage(long pageIndex_);
  void selectNextEntry();
  void selectPrevious();
  void jumpToNextPage();
  void jumpToPreviousPage();

  // printout
  static std::string askQuestion(
      const std::string& question_, const std::vector<std::string>& answers_,
      const std::vector<std::vector<std::string>>& descriptions_= {}
  );

protected:
  void invalidateCache() const;
  void refillPageEntryCache() const;

private:
  // user parameters
  std::string _cursorMarker_{">"};

  // selector data
  size_t _cursorPosition_{0};
  MenuLineList _header_{};
  MenuLineList _footer_{};
  std::vector<SelectorEntry> _entryList_{};

  // caches
  mutable bool _isPageEntryCacheValid_{false};
  mutable std::vector<std::vector<size_t>> _pageEntryCache_{}; // _entryPageMap_[iPage][iEntry] = entryIndex;
  u64 _previousKheld_{0};
  u64 _holdingTiks_{0};

  // dummies
  static const SelectorEntry _dummyEntry_;

};


#endif //SMM_CORE_SELECTOR_H
