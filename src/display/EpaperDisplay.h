#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>

namespace display {

constexpr int8_t kEpdCs = 27;
constexpr int8_t kEpdDc = 14;
constexpr int8_t kEpdRst = 12;
constexpr int8_t kEpdBusy = 13;

using Panel =
    GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT>;  // Wokwi 2.9"

class EpaperDisplay {
 public:
  EpaperDisplay();

  void begin();
  Panel& panel();

  template <typename DrawFn>
  void drawFrame(DrawFn&& drawFn, bool fullRefresh) {
    if (fullRefresh) {
      panel_.setFullWindow();
    } else {
      panel_.setPartialWindow(0, 0, panel_.width(), panel_.height());
    }

    panel_.firstPage();
    do {
      drawFn(panel_);
    } while (panel_.nextPage());
  }

 private:
  Panel panel_;
};

}  // namespace display
