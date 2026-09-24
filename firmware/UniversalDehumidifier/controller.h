#pragma once
#include <cstdint>
#include "app_types.h"
#include "compressor_guard.h"
#include "config.h"

class DehumidifierController {
 public:
  explicit DehumidifierController(const ControlConfig& cfg);
  void reset(std::uint32_t nowMs);
  OutputRequest update(std::uint32_t nowMs, const SensorSnapshot& sensors, const InputSnapshot& inputs);
  OperatingState state() const { return state_; }
 private:
  bool criticalFault(const SensorSnapshot& sensors, const InputSnapshot& inputs) const;
  bool humidityDemand(const SensorSnapshot& sensors) const;
  const ControlConfig& cfg_;
  CompressorGuard guard_;
  OperatingState state_ = OperatingState::BOOT;
  bool demandLatched_ = false, pumpRunning_ = false;
  std::uint32_t stateSinceMs_ = 0, pumpSinceMs_ = 0;
};
