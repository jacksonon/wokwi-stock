#pragma once

#include <Arduino.h>
#include <math.h>

namespace core {

struct StockQuote {
  String symbol;
  String code;
  String name;

  float last = NAN;
  float prevClose = NAN;
  float open = NAN;
  float high = NAN;
  float low = NAN;
  float change = NAN;
  float changePct = NAN;

  uint32_t volume = 0;
  float amountWan = NAN;

  String timestampRaw;
  String timestampDisplay;

  bool isValid() const { return !isnan(last); }

  bool differsForDisplay(const StockQuote& other, float priceEps = 0.0005f,
                         float pctEps = 0.01f) const {
    if (symbol != other.symbol || code != other.code || name != other.name ||
        timestampDisplay != other.timestampDisplay) {
      return true;
    }

    if (isnan(last) != isnan(other.last) ||
        (!isnan(last) && fabsf(last - other.last) > priceEps)) {
      return true;
    }

    if (isnan(change) != isnan(other.change) ||
        (!isnan(change) && fabsf(change - other.change) > priceEps)) {
      return true;
    }

    if (isnan(changePct) != isnan(other.changePct) ||
        (!isnan(changePct) && fabsf(changePct - other.changePct) > pctEps)) {
      return true;
    }

    if (isnan(high) != isnan(other.high) ||
        (!isnan(high) && fabsf(high - other.high) > priceEps)) {
      return true;
    }

    if (isnan(low) != isnan(other.low) ||
        (!isnan(low) && fabsf(low - other.low) > priceEps)) {
      return true;
    }

    return volume != other.volume;
  }
};

}  // namespace core
