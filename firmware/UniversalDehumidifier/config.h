#pragma once
#include <cstdint>

struct ControlConfig {
  float targetRh = 50.0f;
  float hysteresisRh = 2.0f;
  std::uint32_t startupLockoutMs = 180000UL;
  std::uint32_t compressorMinOffMs = 180000UL;
  std::uint32_t compressorMinOnMs = 60000UL;
  std::uint32_t fanPrerunMs = 5000UL;
  std::uint32_t fanPostrunMs = 60000UL;
  std::uint32_t pumpMaxRunMs = 120000UL;
  bool pumpEnabled = false;
  bool requireEvapSensor = true;
  float freezeThresholdC = 2.0f;
  float freezeRecoveryC = 8.0f;
};
