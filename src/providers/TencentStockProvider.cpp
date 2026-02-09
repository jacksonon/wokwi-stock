#include "TencentStockProvider.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <math.h>

namespace {

float toFloatOrNaN(const String& value) {
  if (value.length() == 0) {
    return NAN;
  }

  char* end = nullptr;
  const float v = strtof(value.c_str(), &end);
  if (end == value.c_str()) {
    return NAN;
  }
  return v;
}

uint32_t toUintOrZero(const String& value) {
  if (value.length() == 0) {
    return 0;
  }

  char* end = nullptr;
  unsigned long parsed = strtoul(value.c_str(), &end, 10);
  if (end == value.c_str()) {
    return 0;
  }

  return static_cast<uint32_t>(parsed);
}

String extractQuotedBody(const String& response) {
  const int firstQuote = response.indexOf('"');
  const int lastQuote = response.lastIndexOf('"');
  if (firstQuote < 0 || lastQuote <= firstQuote) {
    return String();
  }
  return response.substring(firstQuote + 1, lastQuote);
}

}  // namespace

namespace providers {

TencentStockProvider::TencentStockProvider(const String& apiHost)
    : apiHost_(apiHost) {}

bool TencentStockProvider::begin() { return true; }

FetchResult TencentStockProvider::fetch(const String& symbol) {
  FetchResult result;

  if (WiFi.status() != WL_CONNECTED) {
    result.error = "WiFi not connected";
    return result;
  }

  HTTPClient http;
  const String url = apiHost_ + symbol;
  if (!http.begin(url)) {
    result.error = "HTTP begin failed";
    return result;
  }

  http.setTimeout(8000);
  http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");

  const int statusCode = http.GET();
  if (statusCode != HTTP_CODE_OK) {
    result.error = "HTTP status " + String(statusCode);
    http.end();
    return result;
  }

  const String payload = http.getString();
  http.end();

  core::StockQuote quote;
  String parseError;
  if (!parseQuote(symbol, payload, quote, parseError)) {
    result.error = parseError;
    return result;
  }

  result.ok = true;
  result.quote = quote;
  return result;
}

bool TencentStockProvider::parseQuote(const String& symbol, const String& response,
                                      core::StockQuote& out,
                                      String& error) const {
  const String body = extractQuotedBody(response);
  if (body.length() == 0) {
    error = "Malformed payload";
    return false;
  }

  String fields[kMaxFields];
  const size_t fieldCount = splitFields(body, fields, kMaxFields);
  if (fieldCount < 35) {
    error = "Field count too small: " + String(fieldCount);
    return false;
  }

  out.symbol = symbol;
  out.name = fields[1];
  out.code = fields[2].length() ? fields[2] : symbol;
  if (!isMostlyAscii(out.name)) {
    out.name = out.code;
  }

  out.last = toFloatOrNaN(fields[3]);
  out.prevClose = toFloatOrNaN(fields[4]);
  out.open = toFloatOrNaN(fields[5]);
  out.volume = toUintOrZero(fields[6]);

  out.change = (fieldCount > 31) ? toFloatOrNaN(fields[31]) : NAN;
  if (isnan(out.change) && !isnan(out.last) && !isnan(out.prevClose)) {
    out.change = out.last - out.prevClose;
  }

  out.changePct = (fieldCount > 32) ? toFloatOrNaN(fields[32]) : NAN;
  if (isnan(out.changePct) && !isnan(out.change) && !isnan(out.prevClose) &&
      out.prevClose != 0) {
    out.changePct = (out.change / out.prevClose) * 100.0f;
  }

  out.high = (fieldCount > 33) ? toFloatOrNaN(fields[33]) : NAN;
  out.low = (fieldCount > 34) ? toFloatOrNaN(fields[34]) : NAN;
  out.amountWan = (fieldCount > 37) ? toFloatOrNaN(fields[37]) : NAN;

  out.timestampRaw = (fieldCount > 30) ? fields[30] : "";
  out.timestampDisplay = normalizeTimestamp(out.timestampRaw);

  if (!out.isValid()) {
    error = "Invalid last price";
    return false;
  }

  return true;
}

size_t TencentStockProvider::splitFields(const String& body, String* outFields,
                                         size_t maxFields) {
  size_t count = 0;
  int start = 0;

  while (count < maxFields && start <= body.length()) {
    const int sep = body.indexOf('~', start);
    if (sep < 0) {
      outFields[count++] = body.substring(start);
      break;
    }

    outFields[count++] = body.substring(start, sep);
    start = sep + 1;
  }

  return count;
}

bool TencentStockProvider::isMostlyAscii(const String& value) {
  for (size_t i = 0; i < value.length(); ++i) {
    const unsigned char c = static_cast<unsigned char>(value[i]);
    if (c < 32 || c > 126) {
      return false;
    }
  }
  return true;
}

String TencentStockProvider::normalizeTimestamp(const String& rawTs) {
  if (rawTs.length() < 14) {
    return rawTs;
  }

  return rawTs.substring(0, 4) + "-" + rawTs.substring(4, 6) + "-" +
         rawTs.substring(6, 8) + " " + rawTs.substring(8, 10) + ":" +
         rawTs.substring(10, 12) + ":" + rawTs.substring(12, 14);
}

}  // namespace providers
