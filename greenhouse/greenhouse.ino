#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <microDS18B20.h>
#include <RTClib.h>
#include <iarduino_I2C_TDS.h>
#include <iarduino_I2C_pH.h>
#include <Adafruit_PCF8574.h>

const char* ssid = "Адрес сети"; // адрес сети wifi
const char* password = "Пароль сети"; // пароль сети wifi
uint8_t temp1_addr[] = { 0x28, 0xD7, 0xDA, 0x49, 0xF6, 0xE0, 0x3C, 0xFB }; // адрес первого термометра
uint8_t temp2_addr[] = { 0x28, 0x8E, 0x9, 0x49, 0xF6, 0x82, 0x3C, 0x98 }; // адрес второго термометра
int greenhouseID = 2; // id контроллера

#define phAddr 0x09
#define ecAddr 0x0A
#define pcfAddr 0x20

String url = "http://greentech-eco.online";

WiFiClient wfcl;
HTTPClient http;
StaticJsonDocument<200> body;
char json_str[272];
byte tries = 10;
int httpCode;
String payload;

MicroDS18B20<D3, temp1_addr> temp1_sensor;
MicroDS18B20<D3, temp2_addr> temp2_sensor;
RTC_DS1307 rtc;
iarduino_I2C_pH phSensor(phAddr);
iarduino_I2C_TDS ecSensor(ecAddr);
Adafruit_PCF8574 pcf;

double temp1;
double temp2;
double ec;
double ph;
float co2;
int time_now;

int prevVal = LOW;
long th, tl, h, l, ppm;

void setup() {
  pinMode(D4, OUTPUT);
  pinMode(D5, INPUT);
  digitalWrite(D4, HIGH);
  Serial.begin(115200);
  initModules();
  wfcl.setTimeout(10000);
  connectWifi();
  delay(1000);
  createGreenhouse();
}

void loop() {
  get_values();
  sendValues();
}

void get_values() {
  temp1_sensor.requestTemp();  // Запрашиваем преобразование температуры
  temp2_sensor.requestTemp();
  ph = phSensor.getPH();
  ec = getEC();
  co2 = getPPM();
  if (temp1_sensor.readTemp()) temp1 = temp1_sensor.getTemp();
  else temp1 = -1000;
  if (temp2_sensor.readTemp()) temp2 = temp2_sensor.getTemp();
  else temp2 = -1000;
  time_now = rtc.now().unixtime();
}

void print_values() {
  Serial.print("t1 = ");
  Serial.print(temp1);
  Serial.print("  t2 = ");
  Serial.print(temp2);
  Serial.print("  pH = ");
  Serial.print(ph);
  Serial.print("  ec = ");
  Serial.print(ec);
  Serial.print("  CO2 = ");
  Serial.print(co2);
  Serial.print("  time_now = ");
  Serial.println(time_now);
}

double getEC() {
  ecSensor.set_t(21.0f);
  // если высокое напряжение на выходе, то вероятно датчик отключился
  if (ecSensor.getVout() > 1.6) return -1000;
  float ec = ecSensor.getEC();
  ec *= 1.2;     //поправка на отклонение в меньшую сторону
  ec /= 1000.0;  //приводим к нормальному виду
  return ec;
}

int getPPM() {
  bool checkedPPM = true;
  while (true) {
    long tt = millis();
    int myVal = digitalRead(D5);
    if (myVal == HIGH) {
      if (myVal != prevVal) {
        h = tt;
        tl = h - l;

        prevVal = myVal;
      }
    } else {
      if (tt - h > 2000) {
        delay(1000);
        return -1000;
      }
      if (myVal != prevVal) {
        l = tt;
        th = l - h;
        prevVal = myVal;
        ppm = 5000 * (th - 2) / (th + tl - 4);
        checkedPPM = false;
        return ppm;
      }
    }
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);

  while (--tries && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Non Connecting to WiFi..");
    exception(500, 500);
  } else {
    Serial.println("");
    Serial.println("WiFi connected");
  }
}

int createGreenhouse() {
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(wfcl, url + "/microChip/create?id=" + String(greenhouseID));
    int httpCode = http.POST("");
    if (httpCode != 200) {        
      Serial.println(http.getString());
    }
    http.end();
  }
  return 0;
}

int sendValues() {
  if (WiFi.status() == WL_CONNECTED) {  //Проверяем состояние подключения Wi-Fi

    // Заполняем данные
    body["greenHouseId"] = greenhouseID;
    body["temperature1"] = temp1;
    body["temperature2"] = temp2;
    body["ec"] = ec;
    body["co2"] = co2;
    body["ph"] = ph;
    serializeJsonPretty(body, json_str);  //Закидываем в json
    serializeJsonPretty(body, Serial);
    Serial.println(url + "/microChip/processParameters");
    http.begin(wfcl, url + "/microChip/processParameters");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json_str);
    if (httpCode == 200) {  //Проверьте код возврата

      StaticJsonDocument<170> responce;
      DeserializationError error = deserializeJson(responce, http.getString());
      Serial.println("7");
      if (error) {
        Serial.println(error.c_str());
        deviceControl(0, 0, 0, 0, 0, 0);
      } else {
        deviceControl(responce["heat"], responce["vent"], responce["pH"], responce["ec"], responce["light"], responce["watering"]);
        Serial.println("8");
      }
      responce.clear();
      Serial.println("9");
    } else {
      Serial.print("http code: ");
      Serial.println(httpCode);
      Serial.println(http.getString());
      deviceControl(0, 0, 0, 0, 0, 0);
    }
  }
  http.end();
  body.clear();
  return 0;
}

void deviceControl(bool heat, bool vent, bool pH, bool ec, bool light, bool watering) {
  pcf.digitalWrite(1, !heat);
  pcf.digitalWrite(2, !vent);
  pcf.digitalWrite(3, !pH);
  pcf.digitalWrite(4, !ec);
  pcf.digitalWrite(5, !light);
  pcf.digitalWrite(6, !watering);
}

void initModules() {
  if (!pcf.begin(pcfAddr, &Wire)) {
    Serial.println("Реле не работает");
    exception(100, 100);
  }
  pcf.pinMode(1, OUTPUT);
  pcf.pinMode(2, OUTPUT);
  pcf.pinMode(3, OUTPUT);
  pcf.pinMode(4, OUTPUT);
  pcf.pinMode(5, OUTPUT);
  pcf.pinMode(6, OUTPUT);
  deviceControl(0, 0, 0, 0, 0, 0);
  if (!phSensor.begin()) {
    Serial.println("PH не работает");
    exception(100, 100);
  }
  if (!ecSensor.begin()) {
    Serial.println("EC не работает");
    exception(100, 100);
  }
  if (!rtc.begin()) {
    Serial.println("Часы не работают");
    exception(100, 100);
  }
  temp1_sensor.requestTemp();
  temp2_sensor.requestTemp();
  delay(1000);
  if (!temp1_sensor.readTemp() || !temp2_sensor.readTemp()) {
    Serial.println("Датчики температуры не работают");
    exception(100, 100);
  }
}

void exception(int intHigh, int intLow) {
  while(1) {
    digitalWrite(D4, HIGH);
    delay(intHigh);
    digitalWrite(D4, LOW);
    delay(intLow);
  }
}