#include "StockFeature.h"

#include <math.h>

namespace {

String formatFloatOrDash(float value, uint8_t decimals) {
  if (isnan(value)) {
    return "--";
  }
  return String(value, static_cast<unsigned int>(decimals));
}

String formatSigned(float value, uint8_t decimals, const char* suffix = "") {
  if (isnan(value)) {
    return "--";
  }

  char buf[24];
  String fmt = "%+." + String(decimals) + "f";
  snprintf(buf, sizeof(buf), fmt.c_str(), value);
  return String(buf) + suffix;
}

String shortTime(const String& fullTs) {
  if (fullTs.length() >= 19) {
    return fullTs.substring(11);
  }
  return fullTs;
}

}  // namespace

namespace features {

const char* StockFeature::id() const { return "stock"; }

void StockFeature::onEnter(const app::AppState& /*state*/) { dirty_ = true; }

void StockFeature::onTick(const app::AppState& /*state*/, uint32_t /*nowMs*/) {}

bool StockFeature::needsRender() const { return dirty_; }

void StockFeature::render(display::Panel& panel, const app::AppState& state,
                          uint32_t nowMs) {
  panel.fillScreen(GxEPD_WHITE);
  panel.setTextColor(GxEPD_BLACK);

  panel.setTextSize(1);
  panel.setCursor(4, 12);
  panel.print("ESP32 E-Ink Stock");

  panel.setCursor(220, 12);
  panel.print(state.wifiConnected ? "WiFi:OK" : "WiFi:--");
  panel.drawFastHLine(0, 16, panel.width(), GxEPD_BLACK);

  if (!state.hasQuote) {
    panel.setCursor(4, 38);
    panel.print("Waiting for quote...");

    if (state.lastError.length()) {
      panel.setCursor(4, 56);
      panel.print("ERR:");
      panel.setCursor(4, 70);
      panel.print(state.lastError);
    }

    panel.setCursor(4, 116);
    panel.print("Symbol: sz000001");
    return;
  }

  const core::StockQuote& q = state.quote;

  panel.setCursor(4, 30);
  panel.print(q.code.length() ? q.code : q.symbol);
  panel.print(" ");
  panel.print(q.name.length() ? q.name : "-");

  panel.setTextSize(3);
  panel.setCursor(4, 68);
  panel.print(formatFloatOrDash(q.last, 2));

  panel.setTextSize(1);
  panel.setCursor(4, 88);
  panel.print("chg ");
  panel.print(formatSigned(q.change, 2));
  panel.print(" (");
  panel.print(formatSigned(q.changePct, 2, "%"));
  panel.print(")");

  panel.setCursor(4, 102);
  panel.print("H ");
  panel.print(formatFloatOrDash(q.high, 2));
  panel.print("  L ");
  panel.print(formatFloatOrDash(q.low, 2));

  panel.setCursor(4, 116);
  panel.print("Vol ");
  panel.print(String(q.volume));

  uint32_t secLeft = 0;
  if (state.nextFetchDueMs > nowMs) {
    secLeft = (state.nextFetchDueMs - nowMs + 999) / 1000;
  }

  panel.setCursor(192, 102);
  panel.print("next ");
  panel.print(secLeft);
  panel.print("s");

  panel.setCursor(192, 116);
  panel.print("updt ");
  panel.print(shortTime(q.timestampDisplay));

  if (state.lastError.length()) {
    panel.setCursor(192, 88);
    panel.print("ERR");
  }
}

void StockFeature::markDirty() { dirty_ = true; }

void StockFeature::clearDirty() { dirty_ = false; }

}  // namespace features
