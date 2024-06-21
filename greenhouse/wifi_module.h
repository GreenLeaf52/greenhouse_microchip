// wifi_module.h
#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include "devices_state.h"


class WiFiModule {
public:
  WiFiModule(const char* ssid, const char* password, const char* url, int ghID)
    : _ssid(ssid), _password(password), _url(url) {
    _ghID = ghID;
  };
  bool connectToWifi() {
    Serial.println(_ssid);
    Serial.println(_password);
    wfcl.setTimeout(10000);
    WiFi.setAutoReconnect(false);
    WiFi.begin(_ssid, _password);

    _tries = 50;
    while (--_tries>0 && WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Non Connecting to WiFi..");
      return false;
    } else {
      Serial.println("");
      return true;
    }
  }
  bool createGreenhouse() {
    if (WiFi.status() != WL_CONNECTED) return false;
    http.begin(wfcl, String(_url) + "/api/v1/microChips?greenHouseId=" + String(_ghID));
    wfcl.setInsecure();
    int httpCode = http.POST("");
    Serial.println(httpCode);
    if (httpCode != 200) {
      Serial.println(http.getString());
      false;
    }
    http.end();
    return 1;
  }
  DevicesState sendValues(String json) {
    if (WiFi.status() != WL_CONNECTED) return {0, 0, 0, 0, 0, 0, -1};
    http.begin(wfcl, String(_url) + "/api/v1/microChips/processParameters?greenHouseId=" + String(_ghID));
    wfcl.setInsecure();
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    if (httpCode == 200) {
      StaticJsonDocument<170> responce;
      DeserializationError error = deserializeJson(responce, http.getString());
      if (error) {
        Serial.println(error.c_str());
        return {0, 0, 0, 0, 0, 0, -10};
      }
      return { responce["heat"], responce["vent"], responce["pH"], responce["ec"], responce["light"], responce["watering"], 200};
      responce.clear();
    } else {
      Serial.print("http code: ");
      Serial.println(httpCode);
      Serial.println(http.getString());
      return {0, 0, 0, 0, 0, 0, httpCode};
    }
  }
private:
  const char* _ssid;
  const char* _password;
  const char* _url;
  int _tries = 10;
  int _ghID;
  WiFiClientSecure wfcl;
  HTTPClient http;
};

#endif  // WIFI_MODULE_H