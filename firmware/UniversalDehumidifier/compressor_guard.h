#pragma once
#include <cstdint>
#include "config.h"

class CompressorGuard {
 public:
  explicit CompressorGuard(const ControlConfig& cfg) : cfg_(cfg) {}
  void reset(std::uint32_t nowMs);
  bool update(bool demand, bool safetyTrip, std::uint32_t nowMs);
  bool running() const { return running_; }
  std::uint32_t offElapsed(std::uint32_t nowMs) const { return nowMs - lastOffMs_; }
  std::uint32_t onElapsed(std::uint32_t nowMs) const { return nowMs - lastOnMs_; }
 private:
  const ControlConfig& cfg_;
  bool running_ = false;
  std::uint32_t bootMs_ = 0, lastOnMs_ = 0, lastOffMs_ = 0;
};
