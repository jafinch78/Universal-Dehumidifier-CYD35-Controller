#include "compressor_guard.h"

void CompressorGuard::reset(std::uint32_t nowMs) {
  running_ = false; bootMs_ = nowMs; lastOnMs_ = nowMs; lastOffMs_ = nowMs;
}

bool CompressorGuard::update(bool demand, bool safetyTrip, std::uint32_t nowMs) {
  if (safetyTrip) {
    if (running_) { running_ = false; lastOffMs_ = nowMs; }
    return false;
  }
  if (running_) {
    if (!demand && onElapsed(nowMs) >= cfg_.compressorMinOnMs) { running_ = false; lastOffMs_ = nowMs; }
    return running_;
  }
  const bool startupExpired = (nowMs - bootMs_) >= cfg_.startupLockoutMs;
  const bool minOffExpired = offElapsed(nowMs) >= cfg_.compressorMinOffMs;
  if (demand && startupExpired && minOffExpired) { running_ = true; lastOnMs_ = nowMs; }
  return running_;
}
