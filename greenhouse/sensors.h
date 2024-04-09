// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>                            
#include <SparkFunCCS811.h> 
#include <microDS18B20.h>
#include <RTClib.h>
#include <iarduino_I2C_TDS.h>
#include <iarduino_I2C_pH.h>

#include "values.h"

class Sensors {
public:
  Sensors(
    const uint8_t temp1_addr[8],
    const uint8_t temp2_addr[8],
    uint8_t phAddr,
    uint8_t ecAddr,
    uint8_t co2Addr
    )
    : phSensor(phAddr), ecSensor(ecAddr), co2Sensor(co2Addr) {
    memcpy(_temp1_addr, temp1_addr, sizeof(_temp1_addr));
    memcpy(_temp2_addr, temp2_addr, sizeof(_temp2_addr));
  }
  Values get() {
    temp1_sensor.requestTemp();  // Запрашиваем преобразование температуры
    temp2_sensor.requestTemp();
    if (co2Sensor.dataAvailable()) {
      co2Sensor.readAlgorithmResults();
      _co2 = co2Sensor.getCO2();
    } else _co2=-1000;
    _ph = phSensor.getPH();
    _ec = _getEC();
    if (temp1_sensor.readTemp()) _temp1 = temp1_sensor.getTemp();
    else _temp1 = -1000;
    if (temp2_sensor.readTemp()) _temp2 = temp2_sensor.getTemp();
    else _temp2 = -1000;
    _time_now = rtc.now().unixtime();

    return Values(_temp1, _temp2, _ec, _ph, _co2, _time_now);
  }

  bool init() {
    temp1_sensor.setAddress(_temp1_addr);
    temp2_sensor.setAddress(_temp2_addr);
    bool exc = false;
    if (!phSensor.begin()) {
      Serial.println("PH не работает");
      exc = true;
    }
    if (!ecSensor.begin()) {
      Serial.println("EC не работает");
      exc = true;
    }
    if (!rtc.begin()) {
      Serial.println("Часы не работают");
      exc = true;
    }
    if (!co2Sensor.begin()) {
      Serial.println("CO2 не работает");
      exc = true;
    }
    temp1_sensor.requestTemp();
    temp2_sensor.requestTemp();
    delay(1000);
    if (!temp1_sensor.readTemp() || !temp2_sensor.readTemp()) {
      Serial.println("Датчики температуры не работают");
      exc = true;
    }
    return !exc;
  }
private:
  double _temp1;
  double _temp2;
  double _ec;
  double _ph;
  float _co2;
  int _time_now;
  uint8_t _temp1_addr[8];
  uint8_t _temp2_addr[8];

  MicroDS18B20<D3, DS_ADDR_MODE> temp1_sensor;
  MicroDS18B20<D3, DS_ADDR_MODE> temp2_sensor;
  RTC_DS1307 rtc;
  iarduino_I2C_pH phSensor;
  iarduino_I2C_TDS ecSensor;
  CCS811 co2Sensor;

  double _getEC() {
    ecSensor.set_t(21.0f);
    // если высокое напряжение на выходе, то вероятно датчик отключился
    if (ecSensor.getVout() > 1.6) return -1000;
    double ec = ecSensor.getEC();
    ec *= 1.2;     //поправка на отклонение в меньшую сторону
    ec /= 1000.0;  //приводим к нормальному виду
    return ec;
  }

};

#endif  // SENSORS_H