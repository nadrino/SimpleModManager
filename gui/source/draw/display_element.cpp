//
// Created by Adrien BLANCHET on 20/05/2020.
//

#include "display_element.h"

#include <utility>


draw::display_element::display_element() {

  _is_visible_ = true;
  _title_ = "";

}

void draw::display_element::set_title(std::string title_) {
  _title_ = std::move(title_);
}
