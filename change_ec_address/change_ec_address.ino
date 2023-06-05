#include <iarduino_I2C_TDS.h>
#include <Wire.h>

iarduino_I2C_TDS ec; 
uint8_t newAddress = 0x0A;                          // новый адрес
void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);
  if( ec.begin() ){                                 // Инициируем работу с модулем.
        Serial.print("Найден модуль 0x");
        Serial.println(ec.getAddress(), HEX);       // Выводим текущий адрес модуля.
        if( ec.changeAddress(newAddress) ){         // Меняем адрес модуля на newAddress.
            Serial.print("Адрес изменён на 0x");
            Serial.println(ec.getAddress(),HEX);    // Выводим текущий адрес модуля.
        }else{ 
            Serial.println("Адрес не изменён!");
        }
    }else{     
        Serial.println("Модуль не найден!");
    }                                         
}

void loop() {
}