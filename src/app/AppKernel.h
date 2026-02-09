#pragma once

#include <Arduino.h>

#include "../core/AppTypes.h"
#include "../core/interfaces/IAppFeature.h"
#include "../core/interfaces/IStockProvider.h"
#include "../display/EpaperDisplay.h"

namespace app {

class AppKernel {
 public:
  AppKernel(display::EpaperDisplay& display, providers::IStockProvider& provider,
            features::IAppFeature& feature);

  void begin();
  void loop();

 private:
  static bool elapsed(uint32_t nowMs, uint32_t fromMs, uint32_t intervalMs);

  void ensureWifi(uint32_t nowMs);
  void fetchQuote(bool force);
  void renderIfNeeded(uint32_t nowMs);

  display::EpaperDisplay& display_;
  providers::IStockProvider& provider_;
  features::IAppFeature& feature_;

  AppState state_;
  uint32_t lastWifiRetryAtMs_ = 0;
  uint32_t lastFullRefreshAtMs_ = 0;
  bool firstRender_ = true;
};

}  // namespace app
