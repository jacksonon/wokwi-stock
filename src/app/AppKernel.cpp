#include "AppKernel.h"

#include <WiFi.h>

#include "AppConfig.h"

namespace app {

AppKernel::AppKernel(display::EpaperDisplay& display,
                     providers::IStockProvider& provider,
                     features::IAppFeature& feature)
    : display_(display), provider_(provider), feature_(feature) {
  state_.fetchIntervalMs = kFetchIntervalMs;
}

void AppKernel::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(kWifiSsid, kWifiPassword);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         !elapsed(millis(), start, kWifiConnectTimeoutMs)) {
    delay(250);
  }

  state_.wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (state_.wifiConnected) {
    Serial.print("[wifi] connected, ip=");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[wifi] connect timeout, keep retrying in loop");
    state_.lastError = "WiFi connect timeout";
  }

  provider_.begin();
  feature_.onEnter(state_);
  feature_.markDirty();

  fetchQuote(true);
  renderIfNeeded(millis());
}

void AppKernel::loop() {
  const uint32_t nowMs = millis();

  ensureWifi(nowMs);
  fetchQuote(false);
  feature_.onTick(state_, nowMs);

  if (!feature_.needsRender() &&
      elapsed(nowMs, lastFullRefreshAtMs_, kFullRefreshIntervalMs)) {
    feature_.markDirty();
  }

  renderIfNeeded(nowMs);
  delay(30);
}

bool AppKernel::elapsed(uint32_t nowMs, uint32_t fromMs, uint32_t intervalMs) {
  return static_cast<uint32_t>(nowMs - fromMs) >= intervalMs;
}

void AppKernel::ensureWifi(uint32_t nowMs) {
  const bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected != state_.wifiConnected) {
    state_.wifiConnected = connected;
    feature_.markDirty();
    if (connected) {
      Serial.print("[wifi] reconnected, ip=");
      Serial.println(WiFi.localIP());
      state_.lastError = "";
    } else {
      Serial.println("[wifi] disconnected");
      state_.lastError = "WiFi disconnected";
    }
  }

  if (!state_.wifiConnected && elapsed(nowMs, lastWifiRetryAtMs_, kWifiRetryIntervalMs)) {
    lastWifiRetryAtMs_ = nowMs;
    Serial.println("[wifi] retry begin...");
    WiFi.disconnect(false, false);
    WiFi.begin(kWifiSsid, kWifiPassword);
  }
}

void AppKernel::fetchQuote(bool force) {
  const uint32_t nowMs = millis();
  if (!force && !elapsed(nowMs, state_.lastFetchAtMs, state_.fetchIntervalMs)) {
    return;
  }

  state_.lastFetchAtMs = nowMs;
  state_.nextFetchDueMs = nowMs + state_.fetchIntervalMs;

  if (!state_.wifiConnected) {
    if (state_.lastError != "WiFi disconnected") {
      state_.lastError = "WiFi disconnected";
      feature_.markDirty();
    }
    return;
  }

  providers::FetchResult result = provider_.fetch(kDefaultStockCode);
  if (result.ok) {
    const bool changed = !state_.hasQuote || state_.quote.differsForDisplay(result.quote);

    state_.quote = result.quote;
    state_.hasQuote = true;
    state_.lastSuccessAtMs = nowMs;

    if (changed || state_.lastError.length()) {
      feature_.markDirty();
    }

    state_.lastError = "";
    Serial.printf("[quote] ok %s %.2f (%+.2f%%)\n", state_.quote.code.c_str(),
                  state_.quote.last, state_.quote.changePct);
    return;
  }

  if (state_.lastError != result.error) {
    state_.lastError = result.error;
    feature_.markDirty();
  }

  Serial.printf("[quote] failed: %s\n", result.error.c_str());
}

void AppKernel::renderIfNeeded(uint32_t nowMs) {
  if (!feature_.needsRender()) {
    return;
  }

  const bool fullRefresh =
      firstRender_ || elapsed(nowMs, lastFullRefreshAtMs_, kFullRefreshIntervalMs);

  display_.drawFrame(
      [&](display::Panel& panel) { feature_.render(panel, state_, nowMs); },
      fullRefresh);

  feature_.clearDirty();

  if (fullRefresh) {
    lastFullRefreshAtMs_ = nowMs;
    firstRender_ = false;
  }
}

}  // namespace app
