//
// Created by Nadrino on 21/05/2020.
//

#include "main_layout.h"


main_layout::main_layout() : Layout::Layout()
{

  pu::ui::Color white_color = {255, 255, 255, 255};
  pu::ui::Color background_color = {45, 45, 45, 255};

  this->SetBackgroundColor(background_color);

  // Create the TextBlock instance with the text we want
//  this->helloText = pu::ui::elm::TextBlock::New(300, 300, "Press X to answer my question");
//  helloText->SetColor({255,255,255,255});
//   Add the instance to the layout. IMPORTANT! this MUST be done for them to be used, having them as members is not enough (just a simple way to keep them)
//  this->Add(this->helloText);

  i32 icon_height = 60;
  _app_con_ = pu::ui::elm::Image::New(30,4,"romfs:/images/icon.png");
  _app_con_->SetWidth(_app_con_->GetWidth()*(double(icon_height)/_app_con_->GetHeight()));
  _app_con_->SetHeight(icon_height);
  this->Add(_app_con_);

  _window_title_ = pu::ui::elm::TextBlock::New(130, 38, "SimpleModManager");
//  _window_title_->SetFont("DefaultFont@24");
  _window_title_->SetColor(white_color);
  this->Add(_window_title_);

//  _bottom_title_ = pu::ui::elm::TextBlock::New();

  _upper_line_ = pu::ui::elm::Rectangle::New(30, 87, 1220, 1, white_color, 0);
  this->Add(_upper_line_);

  _bottom_line_ = pu::ui::elm::Rectangle::New(30, this->GetHeight()-88, 1220, 1, white_color, 0);
  this->Add(_bottom_line_);

  _menu_items_ = pu::ui::elm::Menu::New(100, 88, (this->GetWidth() - 30*2), white_color, 54, 10);
  _menu_items_->SetVerticalAlign(pu::ui::elm::VerticalAlign::Center);
  _menu_items_->SetHorizontalAlign(pu::ui::elm::HorizontalAlign::Center);
  _menu_items_->SetColor(background_color);
  _menu_items_->SetOnFocusColor({60, 60, 80, 255});
  _menu_items_->SetScrollbarColor(background_color);
  this->Add(_menu_items_);

}


void main_layout::load_items(std::vector<std::string> items_list_) {

  _menu_items_->ClearItems();

  for(auto& item_name : items_list_){

    auto item = pu::ui::elm::MenuItem::New(item_name);
    item->SetColor({255,255,255,255});
//    item->SetIcon(); // TODO
//    item->AddOnClick(std::bind(&MainLayout::OpenAction, this), KEY_A);
//    _menu_items_->SetItemSize(20);
    _menu_items_->AddItem(item);

  }

  _menu_items_->SetSelectedIndex(0);

}

pu::ui::elm::Menu::Ref &main_layout::getMenuItems() {
  return _menu_items_;
}
