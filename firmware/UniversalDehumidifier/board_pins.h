#pragma once
#include <Arduino.h>
#include <Wire.h>
namespace BoardPins {
constexpr int TFT_BACKLIGHT=27;
constexpr int EXT_I2C_SDA=25;
constexpr int EXT_I2C_SCL=32;
constexpr std::uint32_t EXT_I2C_HZ=100000;
constexpr std::uint16_t TOUCH_CAL[5]={295,3524,310,3487,7};
inline void beginExternalI2C(TwoWire& bus){ bus.begin(EXT_I2C_SDA,EXT_I2C_SCL,EXT_I2C_HZ); }
}
