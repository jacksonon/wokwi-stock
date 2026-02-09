#pragma once

#include <Arduino.h>

#include "../core/interfaces/IAppFeature.h"

namespace features {

class StockFeature : public IAppFeature {
 public:
  const char* id() const override;
  void onEnter(const app::AppState& state) override;
  void onTick(const app::AppState& state, uint32_t nowMs) override;
  bool needsRender() const override;
  void render(display::Panel& panel, const app::AppState& state,
              uint32_t nowMs) override;
  void markDirty() override;
  void clearDirty() override;

 private:
  bool dirty_ = true;
};

}  // namespace features
