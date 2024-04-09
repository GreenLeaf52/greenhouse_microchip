// values.h
#ifndef VALUES_H
#define VALUES_H

#include <ArduinoJson.h>

class Values {
public:
  Values(
    double temp1,
    double temp2,
    double ec,
    double ph,
    float co2,
    int time_now) {
    _temp1 = temp1;
    _temp2 = temp2;
    _ec = ec;
    _ph = ph;
    _co2 = co2;
    _time_now = time_now;
  }
  String toJson() {
    StaticJsonDocument<200> _body;
    _body["time"] = _time_now;
    _body["temperature1"] = _temp1;
    _body["temperature2"] = _temp2;
    _body["ec"] = _ec;
    _body["co2"] = _co2;
    _body["ph"] = _ph;
    String jsonString;
    serializeJsonPretty(_body, jsonString);
    _body.clear();
    return jsonString;
}

private:
  double _temp1;
  double _temp2;
  double _ec;
  double _ph;
  float _co2;
  int _time_now;
};

#endif  // VALUES_H