#pragma once

#include <Arduino.h>

#include "../../display/EpaperDisplay.h"
#include "../AppTypes.h"

namespace features {

class IAppFeature {
 public:
  virtual ~IAppFeature() = default;

  virtual const char* id() const = 0;
  virtual void onEnter(const app::AppState& state) = 0;
  virtual void onTick(const app::AppState& state, uint32_t nowMs) = 0;
  virtual bool needsRender() const = 0;
  virtual void render(display::Panel& panel, const app::AppState& state,
                      uint32_t nowMs) = 0;
  virtual void markDirty() = 0;
  virtual void clearDirty() = 0;
};

}  // namespace features
