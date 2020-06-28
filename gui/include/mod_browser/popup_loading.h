//
// Created by Adrien BLANCHET on 27/06/2020.
//

#ifndef SIMPLEMODMANAGER_POPUP_LOADING_H
#define SIMPLEMODMANAGER_POPUP_LOADING_H

#include <borealis.hpp>
#include <future>

class popup_loading : public brls::View {

public:
  popup_loading();

  void setAsyncResponse(std::future<bool> *asyncResponse);
  void setProgressFraction(double progress_fraction_);
  void setTitle(const std::string &title_);
  void setSubTitle(const std::string &subTitle_);
  void setProgressColor(const NVGcolor &progressColor_);

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;



private:
  double _progress_fraction_;
  NVGcolor _progressColor_;
  std::string _title_;
  std::string _subTitle_;
  std::future<bool>* _asyncResponse_;

};


#endif //SIMPLEMODMANAGER_POPUP_LOADING_H
