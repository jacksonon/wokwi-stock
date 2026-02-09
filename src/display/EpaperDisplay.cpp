#include "EpaperDisplay.h"

namespace display {

EpaperDisplay::EpaperDisplay()
    : panel_(GxEPD2_290_T94_V2(kEpdCs, kEpdDc, kEpdRst, kEpdBusy)) {}

void EpaperDisplay::begin() {
  panel_.init(115200, true, 2, false);
  panel_.setRotation(1);  // 296x128, landscape
  panel_.setTextColor(GxEPD_BLACK);

  panel_.setFullWindow();
  panel_.firstPage();
  do {
    panel_.fillScreen(GxEPD_WHITE);
  } while (panel_.nextPage());
}

Panel& EpaperDisplay::panel() { return panel_; }

}  // namespace display
