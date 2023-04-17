//
// Created by Adrien Blanchet on 17/04/2023.
//

#include "PopupLoadingBox.h"

#include "Logger.h"

#include "thread"
#include "chrono"

LoggerInit([]{
  Logger::setUserHeaderStr("[PopupLoadingBox]");
});


void PopupLoadingBox::pushView(){

  // memory will be handled by brls
  _loadingView_ = new PopupLoadingView();

  // creating a box will make the loading popup resized
  _loadingBox_ = new brls::Dialog( _loadingView_ );

  // make sure the user don't cancel while unfinished
  _loadingBox_->setCancelable( false );

  while( brls::Application::hasViewDisappearing() ){
    // wait for one extra frame before push
    std::this_thread::sleep_for(std::chrono::milliseconds( 16 ));
  }

  // push the box to the view
  LogDebug << "Pushing progress bar to view" << std::endl;
  brls::Application::pushView( _loadingBox_ );
}
void PopupLoadingBox::popView(){
  // call if it's still on top
  if( _loadingBox_ == brls::Application::getTopStackView() ){
    brls::Application::popView( brls::ViewAnimation::FADE );
  }
}

PopupLoadingView *PopupLoadingBox::getLoadingView() const {
  return _loadingView_;
}
