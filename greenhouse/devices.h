// devices.h
#ifndef DEVICES_H
#define DEVICES_H

#include <Adafruit_PCF8574.h>

#include "devices_state.h"
class Devices {
public:
  Devices(uint8_t pcfAddr) {
    _pcfAddr = pcfAddr;
  }
  void control(DevicesState state) {
    pcf.digitalWrite(1, state.heat);
    pcf.digitalWrite(2, state.vent);
    pcf.digitalWrite(3, state.pH);
    pcf.digitalWrite(4, state.ec);
    pcf.digitalWrite(5, state.light);
    pcf.digitalWrite(6, state.watering);
  }
  bool init() {
    if (!pcf.begin(_pcfAddr, &Wire)) {
      Serial.println("Реле не работает");
      return false;
    }
    pcf.pinMode(1, OUTPUT);
    pcf.pinMode(2, OUTPUT);
    pcf.pinMode(3, OUTPUT);
    pcf.pinMode(4, OUTPUT);
    pcf.pinMode(5, OUTPUT);
    pcf.pinMode(6, OUTPUT);
    return true;
  }
private:
  Adafruit_PCF8574 pcf;
  uint8_t _pcfAddr;
};

#endif  // DEVICES_H