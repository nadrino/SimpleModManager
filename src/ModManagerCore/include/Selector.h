//
// Created by Nadrino on 12/09/2019.
//

#ifndef SMM_CORE_SELECTOR_H
#define SMM_CORE_SELECTOR_H


#include <switch/types.h>

#include <vector>
#include <string>
#include "map"

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

  // native setters
  void setMaxItemsPerPage(size_t maxItemsPerPage_);

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

  // non native getters
  const SelectorEntry& getSelectedEntry() const;
  const std::string& getSelectedEntryTitle() const;
  size_t getCursorPage() const;
  size_t getNbPages() const;
  bool isSelectedEntry(const SelectorEntry& entry_) const;

  // io
  void print() const;
  void scanInputs(u64 kDown, u64 kHeld);

  // cursor moving
  void moveCursorPosition(long cursorPosition_);
  void jumpToPage(long pageIndex_);
  void selectNextEntry();
  void selectPrevious();
  void jumpToNextPage();
  void jumpToPreviousPage();

  // printout
  static std::string ask_question(
      const std::string& question_, const std::vector<std::string>& answers_,
      const std::vector<std::vector<std::string>>& descriptions_={}
  );

protected:
  void invalidateCache() const;
  void refillPageEntryCache() const;

private:
  // user parameters
  size_t _maxItemsPerPage_{30};
  std::string _cursorMarker_{">"};

  // selector data
  size_t _cursorPosition_{0};
  std::vector<SelectorEntry> _entryList_;

  // caches
  mutable bool _isPageEntryCacheValid_{false};
  mutable std::vector<std::vector<size_t>> _pageEntryCache_{}; // _entryPageMap_[iPage][iEntry] = entryIndex;
  u64 _previousKheld_{0};
  u64 _holdingTiks_{0};

  // dummies
  static const SelectorEntry _dummyEntry_;

};


#endif //SMM_CORE_SELECTOR_H
