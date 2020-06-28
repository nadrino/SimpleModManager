//
// Created by Adrien BLANCHET on 27/06/2020.
//

#include "popup_loading.h"
#include <future>
#include <borealis.hpp>
#include <chrono>

using namespace std::literals::chrono_literals;

popup_loading::popup_loading() {

  _progress_fraction_ = 0;
  _title_ = "_title_";
  _subTitle_ = "_subTitle_";
  _asyncResponse_ = nullptr;
  _progressColor_ = nvgRGB(0x00, 0xff, 0xc8);

}

void popup_loading::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {

  // Draw text
  nvgFillColor(vg, a(ctx->theme->textColor));
  nvgFontSize(vg, style->Label.dialogFontSize);
  nvgFontFaceId(vg, ctx->fontStash->regular);
  nvgTextLineHeight(vg, 1.0f);
  nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgBeginPath(vg);
  nvgText(vg, x + width / 2, y + height / 2 - 2*style->Label.dialogFontSize, _title_.c_str(), nullptr);

  // Draw text
  nvgFillColor(vg, a(ctx->theme->textColor));
  nvgFontSize(vg, style->Label.regularFontSize);
  nvgFontFaceId(vg, ctx->fontStash->regular);
  nvgTextLineHeight(vg, 1.0f);
  nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgBeginPath(vg);
  nvgText(vg, x + width / 2, y + height / 2, _subTitle_.c_str(), nullptr);

  const unsigned barLength = width - 30;
  unsigned progress_y = y + 0.8f * height;

  /* Progress bar background */
  /* In official software this is more trasparent instead of brighter. */
  nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
  nvgBeginPath(vg);
  nvgRoundedRect(vg, x + 15, progress_y, barLength, 16, 8);
  nvgFill(vg);

  /* Progress bar */
  nvgFillColor(vg, a(_progressColor_));
  nvgBeginPath(vg);
  nvgRoundedRect(vg, x + 15, progress_y, barLength * _progress_fraction_, 16, 8);
  nvgFill(vg);

}

void popup_loading::setProgressFraction(double progress_fraction_) {
  _progress_fraction_ = progress_fraction_;
}

void popup_loading::setTitle(const std::string &title_) {
  _title_ = title_;
}

void popup_loading::setSubTitle(const std::string &subTitle_) {
  _subTitle_ = subTitle_;
}

void popup_loading::setAsyncResponse(std::future<bool> *asyncResponse) {
  _asyncResponse_ = asyncResponse;
}

void popup_loading::setProgressColor(const NVGcolor &progressColor_) {
  _progressColor_ = progressColor_;
}
