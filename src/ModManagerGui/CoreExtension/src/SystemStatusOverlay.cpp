#include "SystemStatusOverlay.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>
#include <chrono>

#include <switch.h>

namespace {

constexpr int kStatusAreaWidth = 230;
constexpr int kStatusRightPadding = 8;
constexpr int kStatusFontSize = 22;

bool g_psmInitAttempted = false;
bool g_psmReady = false;
std::string g_statusText = "--:--";
std::chrono::steady_clock::time_point g_lastRefresh{};

void ensurePsmInitialized(){
  if( g_psmInitAttempted ){
    return;
  }

  g_psmReady = R_SUCCEEDED(psmInitialize());
  g_psmInitAttempted = true;
}

bool fetchCalendarTime(TimeCalendarTime& outCalendar_){
  u64 timestamp = 0;
  if( R_SUCCEEDED(timeGetCurrentTime(TimeType_Default, &timestamp)) ){
    TimeCalendarAdditionalInfo additionalInfo{};
    if( R_SUCCEEDED(timeToCalendarTimeWithMyRule(timestamp, &outCalendar_, &additionalInfo)) ){
      return true;
    }
  }

  const std::time_t rawTime = std::time(nullptr);
  const std::tm* localTime = std::localtime(&rawTime);
  if( localTime == nullptr ){
    return false;
  }

  outCalendar_.hour = static_cast<u8>(localTime->tm_hour);
  outCalendar_.minute = static_cast<u8>(localTime->tm_min);
  return true;
}

void refreshStatusText(){
  const auto now = std::chrono::steady_clock::now();
  if( not g_statusText.empty()
      and g_lastRefresh.time_since_epoch().count() != 0
      and now - g_lastRefresh < std::chrono::seconds(1) ){
    return;
  }
  g_lastRefresh = now;

  char timeBuffer[8] = "--:--";
  TimeCalendarTime calendarTime{};
  if( fetchCalendarTime(calendarTime) ){
    std::snprintf(timeBuffer, sizeof(timeBuffer), "%02u:%02u",
        static_cast<unsigned>(calendarTime.hour),
        static_cast<unsigned>(calendarTime.minute));
  }

  ensurePsmInitialized();

  u32 batteryPercent = 0;
  if( g_psmReady and R_SUCCEEDED(psmGetBatteryChargePercentage(&batteryPercent)) ){
    char statusBuffer[24];
    std::snprintf(statusBuffer, sizeof(statusBuffer), "%s  %u%%",
        timeBuffer,
        static_cast<unsigned>(std::min<u32>(batteryPercent, 100)));
    g_statusText = statusBuffer;
  }
  else{
    g_statusText = timeBuffer;
  }
}

} // namespace

namespace SystemStatusOverlay {

void draw(NVGcontext* vg, int x, int y, unsigned width, brls::Style* style, brls::FrameContext* ctx){
  refreshStatusText();
  if( g_statusText.empty() ){
    return;
  }

  const int statusRight = x + static_cast<int>(width) - static_cast<int>(style->AppletFrame.separatorSpacing) - kStatusRightPadding;
  const int statusLeft = statusRight - kStatusAreaWidth;
  const int statusCenterY = y + static_cast<int>(style->AppletFrame.headerHeightRegular) / 2
      + static_cast<int>(style->AppletFrame.titleOffset);

  nvgSave(vg);

  nvgBeginPath(vg);
  nvgFillColor(vg, ctx->theme->backgroundColorRGB);
  nvgRect(vg, statusLeft, y, kStatusAreaWidth + kStatusRightPadding, style->AppletFrame.headerHeightRegular - 2);
  nvgFill(vg);

  nvgFillColor(vg, ctx->theme->textColor);
  nvgFontFaceId(vg, ctx->fontStash->regular);
  nvgFontSize(vg, kStatusFontSize);
  nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
  nvgBeginPath(vg);
  nvgText(vg, statusRight, statusCenterY, g_statusText.c_str(), nullptr);

  nvgRestore(vg);
}

void shutdown(){
  if( g_psmReady ){
    psmExit();
  }

  g_psmReady = false;
  g_psmInitAttempted = false;
}

}
