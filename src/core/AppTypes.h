#pragma once

#include <Arduino.h>

#include "models/StockQuote.h"

namespace app {

struct AppState {
  bool wifiConnected = false;
  bool hasQuote = false;
  core::StockQuote quote;
  String lastError;

  uint32_t fetchIntervalMs = 60 * 1000;
  uint32_t lastFetchAtMs = 0;
  uint32_t lastSuccessAtMs = 0;
  uint32_t nextFetchDueMs = 0;
};

}  // namespace app
