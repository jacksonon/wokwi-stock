#pragma once

#include <Arduino.h>

#include "../models/StockQuote.h"

namespace providers {

struct FetchResult {
  bool ok = false;
  core::StockQuote quote;
  String error;
};

class IStockProvider {
 public:
  virtual ~IStockProvider() = default;

  virtual bool begin() = 0;
  virtual FetchResult fetch(const String& symbol) = 0;
};

}  // namespace providers
