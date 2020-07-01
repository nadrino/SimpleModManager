//
// Created by Adrien BLANCHET on 27/06/2020.
//

#include "popup_loading.h"
#include <future>
#include <borealis.hpp>
#include <chrono>
#include <utility>

using namespace std::literals::chrono_literals;

popup_loading::popup_loading() {

  this->reset();

}

void popup_loading::reset(){

  _header_ = "";
  _title_ = "";
  _titlePtr_ = nullptr;
  _subTitle_ = "";
  _subTitlePtr_ = nullptr;
  _enableSubLoadingBar_ = false;

  _subProgressFractionPtr_ = nullptr;
  _progressFractionPtr_ = nullptr;
  _subProgressFraction_ = 0;
  _progressFraction_ = 0;

  _subProgressColor_ = nvgRGB(0x00, 0xff, 0xc8);
  _progressColor_ = nvgRGB(0x00, 0xff, 0xc8);

}

void popup_loading::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {

  float y_offset = 0;
  if(not _header_.empty()){
    y_offset += 12;
    // Drawing header
    nvgBeginPath(vg);
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgFontSize(vg, style->Header.fontSize);
    nvgFillColor(vg, a(ctx->theme->textColor));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(vg, x + style->Header.rectangleWidth + style->Header.padding, y - height/2., (_header_).c_str(), nullptr);

    // Separator
    nvgBeginPath(vg);
    nvgFillColor(vg, a(ctx->theme->separatorColor));
    nvgRect(vg, x, y - height/2. + style->Header.fontSize, width, 1);
    nvgFill(vg);
  }

  // Draw Title
  if(_titlePtr_ == nullptr) _titlePtr_ = &_title_;
  nvgFillColor(vg, a(ctx->theme->textColor));
  nvgFontSize(vg, style->Label.dialogFontSize);
  nvgFontFaceId(vg, ctx->fontStash->regular);
  nvgTextLineHeight(vg, 1.0f);
  nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgBeginPath(vg);
  nvgText(vg, x + width / 2, y + y_offset + height / 2 - 1.8*style->Label.dialogFontSize, (*_titlePtr_).c_str(), nullptr);

  // Draw subTitle
  if(_subTitlePtr_ == nullptr) _subTitlePtr_ = &_subTitle_;
  if((*_subTitlePtr_) != ""){ // .empty() does not work
    nvgBeginPath(vg);
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgFontSize(vg, style->Header.fontSize);
    nvgFillColor(vg, a(ctx->theme->textColor));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(vg, x + style->Header.rectangleWidth + style->Header.padding, y + y_offset + height / 2., (*_subTitlePtr_).c_str(), nullptr);
  }


  unsigned x_margin = 15;
  unsigned totalBarLength = width - 2*x_margin;
  unsigned progress_x_offset = x + x_margin;
  unsigned progress_y_offset = y + y_offset + 0.8f * height;

  /* Progress bar background */
  /* In official software this is more trasparent instead of brighter. */
  nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
  nvgBeginPath(vg);
  nvgRoundedRect(vg, float(progress_x_offset), float(progress_y_offset), float(totalBarLength), 16, 8);
  nvgFill(vg);

  if(_progressFractionPtr_ != nullptr) _progressFraction_ = *_progressFractionPtr_;
  if(_progressFraction_ > 1) _progressFraction_ = 1;
  if(_progressFraction_ < 0) _progressFraction_ = 0;

  /* Progress bar */
  nvgFillColor(vg, a(_progressColor_));
  nvgBeginPath(vg);
  nvgRoundedRect(vg, float(progress_x_offset), float(progress_y_offset), totalBarLength * _progressFraction_, 16, 8);
  nvgFill(vg);

  if(_enableSubLoadingBar_){
    if(_subProgressFractionPtr_ != nullptr) _subProgressFraction_ = *_subProgressFractionPtr_;
    if(_subProgressFraction_ > 1) _subProgressFraction_ = 1;
    if(_subProgressFraction_ < 0) _subProgressFraction_ = 0;

    progress_y_offset += 2*16;

    /* Progress bar background */
    /* In official software this is more transparent instead of brighter. */
    nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, float(progress_x_offset), float(progress_y_offset), totalBarLength, 16, 8);
    nvgFill(vg);

    /* Progress bar */
    nvgFillColor(vg, a(_progressColor_));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, float(progress_x_offset), float(progress_y_offset), totalBarLength * _subProgressFraction_, 16, 8);
    nvgFill(vg);
  }

}

void popup_loading::setProgressFraction(double progress_fraction_) {
  _progressFraction_ = progress_fraction_;
}

void popup_loading::setTitle(const std::string &title_) {
  _title_ = title_;
}

void popup_loading::setSubTitle(const std::string &subTitle_) {
  _subTitle_ = subTitle_;
}

void popup_loading::setProgressColor(const NVGcolor &progressColor_) {
  _progressColor_ = progressColor_;
}

void popup_loading::setSubProgressColor(const NVGcolor &subProgressColor) {
  _subProgressColor_ = subProgressColor;
}

void popup_loading::setSubProgressFraction(double subProgressFraction) {
  _subProgressFraction_ = subProgressFraction;
}

void popup_loading::setSubProgressFractionPtr(double *subProgressFractionPtr) {
  _subProgressFractionPtr_ = subProgressFractionPtr;
}

void popup_loading::setProgressFractionPtr(double *progressFractionPtr) {
  _progressFractionPtr_ = progressFractionPtr;
}

void popup_loading::setEnableSubLoadingBar(bool enableSubLoadingBar) {
  _enableSubLoadingBar_ = enableSubLoadingBar;
}

void popup_loading::setTitlePtr(std::string *titlePtr) {
  _titlePtr_ = titlePtr;
}

void popup_loading::setSubTitlePtr(std::string *subTitlePtr) {
  _subTitlePtr_ = subTitlePtr;
}

void popup_loading::setHeader(std::string header) {
  _header_ = std::move(header);
}
