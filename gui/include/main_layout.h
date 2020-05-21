//
// Created by Adrien BLANCHET on 21/05/2020.
//

#ifndef SIMPLEMODMANAGER_MAIN_LAYOUT_H
#define SIMPLEMODMANAGER_MAIN_LAYOUT_H

#include <vector>
#include <string>

#include <pu/Plutonium>


class main_layout : public pu::ui::Layout
{
public:

  main_layout();
  PU_SMART_CTOR(main_layout) // Have ::Ref alias and ::New() static constructor

  void load_items(std::vector<std::string> &items_list_);



private:

  // An easy way to keep objects is to have them as private members
  // Using ::Ref (of a Plutonium built-in object or any class having PU_SMART_CTOR) is an alias to a shared_ptr of the instance.
  pu::ui::elm::TextBlock::Ref helloText;
  pu::ui::elm::TextBlock::Ref _window_title_;
  pu::ui::elm::Rectangle::Ref _upper_line_;
  pu::ui::elm::Rectangle::Ref _bottom_line_;
  pu::ui::elm::Image::Ref _app_con_;

  pu::ui::elm::Menu::Ref _menu_items_;

};


#endif //SIMPLEMODMANAGER_MAIN_LAYOUT_H
