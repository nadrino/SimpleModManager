//
// Created by Adrien BLANCHET on 27/06/2020.
//

#ifndef SIMPLEMODMANAGER_POPUP_LOADING_H
#define SIMPLEMODMANAGER_POPUP_LOADING_H

#include <borealis.hpp>

class popup_loading : public brls::View {

public:
  popup_loading();

  void reset();

  void setEnableSubLoadingBar(bool enableSubLoadingBar);

  void setSubProgressFraction(double subProgressFraction);
  void setSubProgressFractionPtr(double *subProgressFractionPtr);
  void setProgressFractionPtr(double *progressFractionPtr);
  void setProgressFraction(double progress_fraction_);
  void setTitle(const std::string &title_);
  void setSubTitle(const std::string &subTitle_);
  void setSubProgressColor(const NVGcolor &subProgressColor);
  void setProgressColor(const NVGcolor &progressColor_);

  void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;



private:
  bool _enableSubLoadingBar_;

  double _subProgressFraction_;
  double* _subProgressFractionPtr_;
  double _progressFraction_;
  double* _progressFractionPtr_;

  NVGcolor _subProgressColor_;
  NVGcolor _progressColor_;

  std::string _title_;
  std::string _subTitle_;

};


#endif //SIMPLEMODMANAGER_POPUP_LOADING_H
