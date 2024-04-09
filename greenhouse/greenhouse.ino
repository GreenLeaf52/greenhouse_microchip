#include "sensors.h"
#include "devices.h"
#include "wifi_module.h"
#include <Esp.h>
#include <vector>

const char* ssid = "PODVbIPERDbISH";                                         // адрес сети wifi
const char* password = "oralcumshot83cm";                                    // пароль сети wifi
uint8_t temp1_addr[8] = { 0x28, 0xD7, 0xDA, 0x49, 0xF6, 0xE0, 0x3C, 0xFB };  // адрес первого термометра
uint8_t temp2_addr[8] = { 0x28, 0x8E, 0x9, 0x49, 0xF6, 0x82, 0x3C, 0x98 };   // адрес второго термометра
int greenhouseID = 2;                                                        // id контроллера

const char* url = "https://greentech-eco.online";

#define phAddr 0x09
#define ecAddr 0x0A
#define pcfAddr 0x20
#define co2Addr 0x5A

Sensors sensors = Sensors(temp1_addr, temp2_addr, phAddr, ecAddr, co2Addr);
Devices devices = Devices(pcfAddr);
WiFiModule wifiModule = WiFiModule(ssid, password, url, greenhouseID);

std::vector<String> dataQueue;

void setup() {
  pinMode(D4, OUTPUT);
  Serial.begin(115200);
  initial();
}

void loop() {
  Values values = sensors.get();
  Serial.println(values.toJson());
  DevicesState state = wifiModule.sendValues(values.toJson());
  if (state.code == -1) {
      dataQueue.push_back(values.toJson());
      Serial.println("Нет интернета, добавили в массив");
  }
  if (state.code == 200) {
    Serial.println("200");
  } else if (state.code == 502) {
    dataQueue.push_back(values.toJson());
    Serial.println("Серв недоступен, добавили в массив");
  } else if (state.code == -10) {
    Serial.println("Кривой ответ");
  }
  devices.control(state);

  if (dataQueue.size()!=0) {
    sendOldValues();
  }

  delay(50000);
}

void sendOldValues() {
  Serial.print("прокидываем старые значения: ");
  Serial.println(dataQueue.size());
  std::vector<String>::iterator it = dataQueue.begin();
  while (it != dataQueue.end()) {
    DevicesState state = wifiModule.sendValues(*it);
      if (state.code==-1) {
        if (!wifiModule.connectToWifi()) return;
        state = wifiModule.sendValues(*it);
      }
      if (state.code!=502) {
        dataQueue.erase(it);
        Serial.println("Удалили элемент");
      } else {
        Serial.println("Оставили элемент");
        ++it;
      }
    }
  Serial.print("Закончили со старыми значениями: ");
  Serial.println(dataQueue.size());
}

void initial() {
  digitalWrite(D4, HIGH);
  if (!sensors.init()) {
    exception(100, 100);
  } else {
    Serial.println("Датчики готовы");
  }

  if (!devices.init()) {
    exception(500, 500);
  } else {
    Serial.println("Реле готовы");
  }

  if (!wifiModule.connectToWifi()) {
    exception(100, 500);
  } else {
    Serial.println("WiFi подключен");
  }
  devices.control({ 0, 0, 0, 0, 0, 0 });

  if (!wifiModule.createGreenhouse()) {
    exception(500, 100);
  } else {
    Serial.println("Теплица создана");
  }
  delay(1000);
}

void exception(int intHigh, int intLow) {
  int duration = 1000 / (intHigh + intLow) * 300;
  while (--duration > 0) {
    digitalWrite(D4, HIGH);
    delay(intHigh);
    digitalWrite(D4, LOW);
    delay(intLow);
  }
  ESP.reset(); // Перезагружаем плату
}