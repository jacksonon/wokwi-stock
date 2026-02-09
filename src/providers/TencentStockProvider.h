#pragma once

#include <Arduino.h>

#include "../core/interfaces/IStockProvider.h"

namespace providers {

class TencentStockProvider : public IStockProvider {
 public:
  explicit TencentStockProvider(const String& apiHost);

  bool begin() override;
  FetchResult fetch(const String& symbol) override;

 private:
  static constexpr size_t kMaxFields = 80;

  String apiHost_;

  bool parseQuote(const String& symbol, const String& response,
                  core::StockQuote& out, String& error) const;

  static size_t splitFields(const String& body, String* outFields,
                            size_t maxFields);
  static bool isMostlyAscii(const String& value);
  static String normalizeTimestamp(const String& rawTs);
};

}  // namespace providers
