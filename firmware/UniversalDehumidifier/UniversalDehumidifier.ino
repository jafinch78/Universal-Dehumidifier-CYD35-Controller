#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "app_types.h"
#include "board_pins.h"
#include "config.h"
#include "controller.h"

TFT_eSPI tft;
TwoWire ApplianceWire=TwoWire(1);
ControlConfig config;
DehumidifierController controller(config);

void setup(){
  pinMode(BoardPins::TFT_BACKLIGHT,OUTPUT); digitalWrite(BoardPins::TFT_BACKLIGHT,LOW);
  Serial.begin(115200); BoardPins::beginExternalI2C(ApplianceWire);
  tft.init(); tft.setRotation(1); tft.setTouch(const_cast<uint16_t*>(BoardPins::TOUCH_CAL));
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawString("Universal Dehumidifier V0.1",10,10,2);
  tft.drawString("OUTPUTS LOCKED - BENCH MODE",10,40,2);
  digitalWrite(BoardPins::TFT_BACKLIGHT,HIGH);
  controller.reset(millis());
  Serial.println("V0.1 bench skeleton: no appliance relay driver enabled");
}
void loop(){ delay(100); }
