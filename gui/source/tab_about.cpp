//
// Created by Adrien BLANCHET on 20/06/2020.
//

#include "tab_about.h"
#include <borealis.hpp>

tab_about::tab_about() {

//  auto* titleLabel = new brls::Label(brls::LabelStyle::LIST_ITEM, "SimpleModManager", true);
//  titleLabel->setFontSize(36);
//  titleLabel->setHorizontalAlign(NVG_ALIGN_CENTER);
//  this->addView(titleLabel);

  // Subtitle
  auto* shortDescription = new brls::Label(
    brls::LabelStyle::REGULAR,
    "SimpleModManager is an Nintendo Switch homebrew app that helps you to manage mods and config files on your SD card.",
    true
  );
  shortDescription->setHorizontalAlign(NVG_ALIGN_CENTER);
  this->addView(shortDescription);

  auto* table = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
  table->setSpacing(22);
  table->setHeight(260);

  auto* leftBox = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
  leftBox->setSpacing(22);
  leftBox->setWidth(500);
  leftBox->setParent(table);
  leftBox->addView(new brls::Header("Test"));

  auto* rightBox = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
  rightBox->setSpacing(22);
  rightBox->setWidth(200);
  rightBox->setParent(table);
  auto* portrait = new brls::Image("romfs:/images/portrait.jpg");
  portrait->setScaleType(brls::ImageScaleType::SCALE);
  portrait->setHeight(200);
  portrait->setParent(rightBox);
  rightBox->addView(portrait);
  auto* portraitText = new brls::Label(brls::LabelStyle::SMALL, "Author: Nadrino\n A parisian neutrino physicist", true);
  portraitText->setHorizontalAlign(NVG_ALIGN_CENTER);
  rightBox->addView(portraitText);

  table->addView(leftBox);
  table->addView(rightBox);

  this->addView(table);

  this->addView(new brls::Header("Remerciements"));
  brls::Label *links = new brls::Label(
    brls::LabelStyle::SMALL,
    "\uE017  SimpleModManager is powered by Borealis, an hardware accelerated UI library\n" \
         "\uE017  Special thanks to the RetroNX team for their support with Borealis\n",
    true
  );
  // "uE010" -> rectangle with !
  // "uE017" -> heart
  // "uE019" -> right arrow
  // "uE020" to 27 -> tinfoil style squares
  // "uE040" -> power then some buttons (official icons start at something like 70)
  // "uE105" is the last
  this->addView(links);


}
