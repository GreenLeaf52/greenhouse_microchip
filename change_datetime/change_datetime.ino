#include <RTClib.h>

RTC_DS1307 rtc;

void setup () {
  Serial.begin(115200);

  if (! rtc.begin()) {
    Serial.println("Часы не работают");
    Serial.flush();
    while (1);
  }
  Serial.println("Зайдите на сайт https://i-leon.ru/tools/time и вставьте текущее Unix epoch время");
  int t = 0;
  while (t==0) {
  t = Serial.parseInt();
  }
  DateTime initialTime(t);
  rtc.adjust(initialTime);
}

void loop () {
  DateTime now = rtc.now();
  Serial.print("Дата и время: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("  ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print("    ");
  Serial.println(now.unixtime());

  delay(1000);
}
