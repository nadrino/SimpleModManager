//
// Created by Adrien Blanchet on 14/04/2023.
//


#include "SimpleModManager.h"

#include <FrameRoot.h>
#include <ModsMtpServer.h>
#include <SystemStatusOverlay.h>
#include "ConsoleHandler.h"

#include "ConfigHandler.h"
#include "Toolbox.h"

#include "Logger.h"

#include <borealis.hpp>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>
#include <cstdlib>
#include "iostream"

#include "switch.h"


LoggerInit([]{
  Logger::setUserHeaderStr("[SimpleModManager.nro]");
});

namespace {

constexpr int kTouchScreenWidth = 1280;
constexpr int kTouchScreenHeight = 720;
constexpr int kTouchTapSlopPx = 24;
constexpr int kTouchNavigationStepPx = 56;

struct TouchState {
  bool touching{false};
  bool dragging{false};
  int startX{0};
  int startY{0};
  int lastX{0};
  int lastY{0};
  int dragAccumulatorY{0};
};

enum class PendingTouchType {
  None,
  View,
  DialogButton
};

struct PendingTouchSelection {
  PendingTouchType type{PendingTouchType::None};
  brls::View* topView{nullptr};
  brls::View* view{nullptr};
  brls::Key key{brls::Key::A};
  int dialogButtonIndex{-1};
};

PendingTouchSelection gPendingTouchSelection;

void clearPendingTouchSelection() {
  gPendingTouchSelection = PendingTouchSelection();
}

bool isSamePendingView(brls::View* topView_, brls::View* view_) {
  return gPendingTouchSelection.type == PendingTouchType::View
      && gPendingTouchSelection.topView == topView_
      && gPendingTouchSelection.view == view_;
}

bool isSamePendingDialogButton(brls::View* topView_, int buttonIndex_) {
  return gPendingTouchSelection.type == PendingTouchType::DialogButton
      && gPendingTouchSelection.topView == topView_
      && gPendingTouchSelection.dialogButtonIndex == buttonIndex_;
}

void dispatchTouchButton(char button_) {
  brls::Application::onGamepadButtonPressed( button_, false );
}

void giveTouchFocus(brls::View* view_) {
  brls::Application::giveFocus( view_ );
}

bool isPointInsideView(brls::View* view_, int x_, int y_) {
  if( view_ == nullptr || view_->isHidden() || view_->isCollapsed() ){
    return false;
  }

  return x_ >= view_->getX()
      && y_ >= view_->getY()
      && x_ < view_->getX() + static_cast<int>(view_->getWidth())
      && y_ < view_->getY() + static_cast<int>(view_->getHeight());
}

brls::View* findActiveTabView(brls::TabFrame* tabFrame_) {
  if( tabFrame_ == nullptr || tabFrame_->sidebar == nullptr ){
    return nullptr;
  }

  for( size_t i = 0; i < tabFrame_->sidebar->getViewsCount(); ++i ){
    auto* item = dynamic_cast<brls::SidebarItem*>(tabFrame_->sidebar->getChild(i));
    if( item != nullptr && item->isActive() ){
      return item->getAssociatedView();
    }
  }

  return nullptr;
}

brls::View* findTouchableView(brls::View* view_, int x_, int y_) {
  if( !isPointInsideView(view_, x_, y_) ){
    return nullptr;
  }

  if( auto* tabFrame = dynamic_cast<brls::TabFrame*>(view_) ){
    if( auto* sidebarHit = findTouchableView(tabFrame->sidebar, x_, y_) ){
      return sidebarHit;
    }
    if( auto* activeView = findActiveTabView(tabFrame) ){
      if( auto* activeHit = findTouchableView(activeView, x_, y_) ){
        return activeHit;
      }
    }
  }

  if( auto* scrollView = dynamic_cast<brls::ScrollView*>(view_) ){
    if( auto* contentView = scrollView->getContentView() ){
      if( auto* contentHit = findTouchableView(contentView, x_, y_) ){
        return contentHit;
      }
    }
  }

  if( auto* boxLayout = dynamic_cast<brls::BoxLayout*>(view_) ){
    for( size_t i = boxLayout->getViewsCount(); i > 0; --i ){
      if( auto* childHit = findTouchableView(boxLayout->getChild(i - 1), x_, y_) ){
        return childHit;
      }
    }
  }

  return view_->getDefaultFocus() == view_ ? view_ : nullptr;
}

bool touchActionsSortFunc(brls::Action a_, brls::Action b_) {
  if( a_.key == brls::Key::PLUS ){
    return true;
  }
  if( b_.key == brls::Key::A ){
    return true;
  }
  if( b_.key == brls::Key::B && a_.key != brls::Key::A ){
    return true;
  }
  return false;
}

std::vector<brls::Action> collectVisibleActions() {
  std::vector<brls::Action> actions;
  std::set<brls::Key> addedKeys;

  brls::View* focusParent = brls::Application::getCurrentFocus();
  if( focusParent == nullptr ){
    focusParent = brls::Application::getTopStackView();
  }

  while( focusParent != nullptr ){
    for( const auto& action : focusParent->getActions() ){
      if( action.hidden || !action.available || action.hintText.empty() ){
        continue;
      }
      if( addedKeys.find(action.key) != addedKeys.end() ){
        continue;
      }

      addedKeys.insert(action.key);
      actions.emplace_back(action);
    }
    focusParent = focusParent->getParent();
  }

  std::stable_sort(actions.begin(), actions.end(), touchActionsSortFunc);
  return actions;
}

const char* getTouchActionIcon(brls::Key key_) {
  switch( key_ ){
    case brls::Key::A: return "\uE0E0";
    case brls::Key::B: return "\uE0E1";
    case brls::Key::X: return "\uE0E2";
    case brls::Key::Y: return "\uE0E3";
    case brls::Key::L: return "\uE0E4";
    case brls::Key::R: return "\uE0E5";
    case brls::Key::PLUS: return "\uE0EF";
    case brls::Key::MINUS: return "\uE0F0";
    case brls::Key::DLEFT: return "\uE0ED";
    case brls::Key::DUP: return "\uE0EB";
    case brls::Key::DRIGHT: return "\uE0EF";
    case brls::Key::DDOWN: return "\uE0EC";
    default: return "\uE152";
  }
}

float measureTouchHintWidth(const std::string& text_) {
  auto* vg = brls::Application::getNVGContext();
  auto* style = brls::Application::getStyle();
  auto* stash = brls::Application::getFontStash();
  if( vg == nullptr || style == nullptr || stash == nullptr ){
    return static_cast<float>(text_.size() * 12 + 36);
  }

  float bounds[4]{};
  nvgFontSize(vg, style->Label.hintFontSize);
  nvgFontFaceId(vg, stash->regular);
  nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
  nvgTextBounds(vg, 0, 0, text_.c_str(), nullptr, bounds);
  return bounds[2] - bounds[0];
}

bool handleFooterTouch(brls::View* topView_, int x_, int y_) {
  auto* style = brls::Application::getStyle();
  if( style == nullptr ){
    return false;
  }

  const int footerTop = static_cast<int>(brls::Application::contentHeight) - static_cast<int>(style->AppletFrame.footerHeight);
  if( y_ < footerTop ){
    return false;
  }

  auto actions = collectVisibleActions();
  if( actions.empty() ){
    return false;
  }

  float right = static_cast<float>(brls::Application::contentWidth
      - style->AppletFrame.separatorSpacing
      - style->AppletFrame.footerTextSpacing);
  const float spacing = static_cast<float>(style->AppletFrame.footerTextSpacing);

  for( auto it = actions.rbegin(); it != actions.rend(); ++it ){
    const std::string label = std::string(getTouchActionIcon(it->key)) + "  " + it->hintText;
    const float width = measureTouchHintWidth(label) + 16.0f;
    const float left = right - width;

    if( x_ >= left - 8.0f && x_ <= right + 8.0f ){
      clearPendingTouchSelection();
      dispatchTouchButton(static_cast<char>(it->key));
      return true;
    }

    right = left - spacing;
  }

  return false;
}

bool handleDialogTouch(brls::View* topView_, int x_, int y_) {
  if( dynamic_cast<brls::Dialog*>(topView_) == nullptr ){
    return false;
  }

  auto* style = brls::Application::getStyle();
  if( style == nullptr ){
    return false;
  }

  const int buttonHeight = static_cast<int>(style->Dialog.buttonHeight);
  const int frameWidth = static_cast<int>(style->Dialog.width);
  const int frameHeight = static_cast<int>(style->Dialog.height) + buttonHeight;
  const int frameX = static_cast<int>(topView_->getWidth()) / 2 - frameWidth / 2;
  const int frameY = static_cast<int>(topView_->getHeight()) / 2 - frameHeight / 2;
  const int buttonTop = frameY + static_cast<int>(style->Dialog.height);
  const int buttonBottom = static_cast<int>(topView_->getHeight());

  if( x_ < frameX || x_ >= frameX + frameWidth || y_ < buttonTop || y_ >= buttonBottom ){
    return false;
  }

  const int buttonIndex = x_ < frameX + frameWidth / 2 ? 0 : 1;
  dispatchTouchButton(buttonIndex == 0 ? GLFW_GAMEPAD_BUTTON_DPAD_LEFT : GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);

  if( !isSamePendingDialogButton(topView_, buttonIndex) ){
    gPendingTouchSelection.type = PendingTouchType::DialogButton;
    gPendingTouchSelection.topView = topView_;
    gPendingTouchSelection.dialogButtonIndex = buttonIndex;
    return true;
  }

  clearPendingTouchSelection();
  dispatchTouchButton(GLFW_GAMEPAD_BUTTON_A);
  return true;
}

bool handleContentTouch(int x_, int y_) {
  auto* topView = brls::Application::getTopStackView();
  if( topView == nullptr || brls::Application::hasViewDisappearing() ){
    return false;
  }

  if( handleDialogTouch(topView, x_, y_) ){
    return true;
  }
  if( handleFooterTouch(topView, x_, y_) ){
    return true;
  }

  auto* touchedView = findTouchableView(topView, x_, y_);
  if( touchedView == nullptr ){
    return false;
  }

  auto* focusTarget = touchedView->getDefaultFocus();
  if( focusTarget == nullptr ){
    return false;
  }

  if( dynamic_cast<brls::SidebarItem*>(focusTarget) != nullptr ){
    clearPendingTouchSelection();
    brls::Application::giveFocus(focusTarget);
    return true;
  }

  if( !isSamePendingView(topView, focusTarget) || brls::Application::getCurrentFocus() != focusTarget ){
    giveTouchFocus(focusTarget);
    gPendingTouchSelection.type = PendingTouchType::View;
    gPendingTouchSelection.topView = topView;
    gPendingTouchSelection.view = focusTarget;
    return true;
  }

  clearPendingTouchSelection();
  dispatchTouchButton(GLFW_GAMEPAD_BUTTON_A);
  return true;
}

void processTouchInput() {
  HidTouchScreenState touchScreenState{};
  if( hidGetTouchScreenStates(&touchScreenState, 1) == 0 ){
    return;
  }

  static TouchState touchState;
  const bool isTouching = touchScreenState.count > 0;

  if( isTouching ){
    const auto& touch = touchScreenState.touches[0];
    const int x = static_cast<int>(std::lround(
        static_cast<double>(touch.x) * static_cast<double>(brls::Application::contentWidth) / kTouchScreenWidth));
    const int y = static_cast<int>(std::lround(
        static_cast<double>(touch.y) * static_cast<double>(brls::Application::contentHeight) / kTouchScreenHeight));

    if( !touchState.touching ){
      touchState.touching = true;
      touchState.dragging = false;
      touchState.startX = touchState.lastX = x;
      touchState.startY = touchState.lastY = y;
      touchState.dragAccumulatorY = 0;
      return;
    }

    const int totalDx = x - touchState.startX;
    const int totalDy = y - touchState.startY;
    if( !touchState.dragging && (std::abs(totalDx) > kTouchTapSlopPx || std::abs(totalDy) > kTouchTapSlopPx) ){
      clearPendingTouchSelection();
      touchState.dragging = true;
    }

    if( touchState.dragging ){
      touchState.dragAccumulatorY += y - touchState.lastY;
      while( std::abs(touchState.dragAccumulatorY) >= kTouchNavigationStepPx ){
        dispatchTouchButton(touchState.dragAccumulatorY < 0 ? GLFW_GAMEPAD_BUTTON_DPAD_DOWN : GLFW_GAMEPAD_BUTTON_DPAD_UP);
        touchState.dragAccumulatorY += touchState.dragAccumulatorY < 0 ? kTouchNavigationStepPx : -kTouchNavigationStepPx;
      }
    }

    touchState.lastX = x;
    touchState.lastY = y;
    return;
  }

  if( touchState.touching ){
    const bool wasTap = !touchState.dragging
        && std::abs(touchState.lastX - touchState.startX) <= kTouchTapSlopPx
        && std::abs(touchState.lastY - touchState.startY) <= kTouchTapSlopPx;
    if( wasTap ){
      handleContentTouch(touchState.lastX, touchState.lastY);
    }
  }

  touchState = TouchState();
}

}


int main(int argc, char* argv[]){
  LogInfo << "SimpleModManager is starting..." << std::endl;

  Toolbox::ensureModsRootFolder();

  // https://github.com/jbeder/yaml-cpp/wiki/Tutorial
//  YAML::Node config = YAML::LoadFile("config.yaml");
//  if (config["lastLogin"]) {
//    std::cout << "Last logged in: " << config["lastLogin"].as<std::string>() << "\n";
//  }
//  const auto username = config["username"].as<std::string>();
//  const auto password = config["password"].as<std::string>();

  ConfigHandler c;
  if( c.getConfig().useGui ){ runGui(c.getConfig().baseFolder); }
  else{
    const Result nsRc = nsInitialize();
    if( R_SUCCEEDED(nsRc) ) {
      Toolbox::ensureInstalledGameModFolders(c.getConfig().baseFolder);
    }
    else {
      LogError << "nsInitialize Failed: 0x" << std::hex << nsRc << std::dec << std::endl;
    }
    consoleInit(nullptr);
    ConsoleHandler::run();
    consoleExit(nullptr);
    if( R_SUCCEEDED(nsRc) ) {
      nsExit();
    }
  }

  // Exit
  return EXIT_SUCCESS;
}


void runGui(const std::string& modsRootFolder_){
  LogInfo << "Starting GUI..." << std::endl;
  LogThrowIf(R_FAILED(nsInitialize()), "nsInitialize Failed");
  Toolbox::ensureInstalledGameModFolders(modsRootFolder_);

  brls::Logger::setLogLevel(brls::LogLevel::ERROR);

  brls::i18n::loadTranslations("en-US");
  LogThrowIf(not brls::Application::init("SimpleModManager"), "Unable to init Borealis application");
  hidInitializeTouchScreen();

  LogInfo << "Creating root frame..." << std::endl;
  auto* mainFrame = new FrameRoot();

  LogInfo << "Pushing to view" << std::endl;
  brls::Application::pushView( mainFrame );
  mainFrame->registerAction( "", brls::Key::PLUS, []{return true;}, true );
  mainFrame->updateActionHint( brls::Key::PLUS, "" ); // make the change visible

  while( brls::Application::mainLoop() ){
    processTouchInput();
  }

  SystemStatusOverlay::shutdown();
  ModsMtpServer::shutdownForAppExit();
  nsExit();
}
