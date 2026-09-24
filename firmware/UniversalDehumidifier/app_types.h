#pragma once
#include <cstdint>

enum class OperatingState : std::uint8_t { BOOT, STARTUP_LOCKOUT, IDLE, FAN_PRERUN, DEHUMIDIFY, FAN_POSTRUN, BUCKET_FULL, FAULT };

struct SensorSnapshot {
  float roomRh = 0.0f;
  float roomTempC = 0.0f;
  float evapTempC = 0.0f;
  bool roomValid = false;
  bool evapValid = false;
};

struct InputSnapshot {
  bool bucketFull = false;
  bool secondaryHigh = false;
  bool interlockOpen = false;
  bool pumpCall = false;
  bool requiredI2cOk = true;
  bool configValid = false;
};

struct OutputRequest {
  bool compressor = false;
  bool fan = false;
  bool pump = false;
  bool defrostAux = false;
  bool alarm = false;
};
