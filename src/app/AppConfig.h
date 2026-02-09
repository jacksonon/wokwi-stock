#pragma once

#include <Arduino.h>

namespace app {
constexpr char kWifiSsid[] = "Wokwi-GUEST";
constexpr char kWifiPassword[] = "";
constexpr char kDefaultStockCode[] = "sz000001";
constexpr char kTencentApiHost[] = "http://qt.gtimg.cn/q=";

constexpr uint32_t kFetchIntervalMs = 60 * 1000;
constexpr uint32_t kFullRefreshIntervalMs = 5 * 60 * 1000;
constexpr uint32_t kWifiConnectTimeoutMs = 15 * 1000;
constexpr uint32_t kWifiRetryIntervalMs = 5 * 1000;
constexpr uint16_t kHttpTimeoutMs = 8000;
}  // namespace app
