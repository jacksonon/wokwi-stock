#include <Arduino.h>

#include "app/AppConfig.h"
#include "app/AppKernel.h"
#include "display/EpaperDisplay.h"
#include "features/StockFeature.h"
#include "providers/TencentStockProvider.h"

display::EpaperDisplay gDisplay;
providers::TencentStockProvider gProvider(app::kTencentApiHost);
features::StockFeature gStockFeature;
app::AppKernel gApp(gDisplay, gProvider, gStockFeature);

void setup() {
  Serial.begin(115200);
  delay(100);

  gDisplay.begin();
  gApp.begin();
}

void loop() { gApp.loop(); }
