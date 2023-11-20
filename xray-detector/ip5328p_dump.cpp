#include "ip5328p_dump.h"
#include "Arduino.h"
#include "Wire.h"

#define IP5328_DUMP_INT_PIN    9
#define IP5328_DUMP_I2C_ADDR   0x75

#if IP5328_DUMP_ENABLED
uint8_t ip5328_dump_read_byte(uint8_t address) {
  uint8_t v = 0x00;
  
  Wire.beginTransmission(IP5328_DUMP_I2C_ADDR);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(IP5328_DUMP_I2C_ADDR, 1, false);
  while (Wire.available()) {
    v = Wire.read();
  }
  Wire.endTransmission();
  return v;
}

void ip5328_dump_init() {
  pinMode(IP5328_DUMP_INT_PIN, INPUT);

  Wire.begin();
  Wire.setClock(200000);
}

void ip5328_dump() {
  Serial.begin(9600);
  delay(100);
  Serial.println("Firmware ready, starting.");

  ip5328_dump_init();

  while(true) {
    while (digitalRead(IP5328_DUMP_INT_PIN) == LOW);

    Serial.println("IP5328 ready, reading memory....");
  
    Serial.print("   ");
    for (int i = 0; i<=0xF; i++) {
      Serial.print(i, HEX);
      Serial.print("  ");
    }

    Serial.println();
  
    for (int i = 0; i<=0xF; i++) {
      Serial.print(i, HEX);
      Serial.print(" ");
  
      for (int j = 0; j<=0xF; j++) {
        uint8_t v = ip5328_dump_read_byte(i * 0xF + j);
        if (v <= 0xF) {
          Serial.print("0");
        }
        Serial.print(v, HEX);
        Serial.print(" ");
      }
  
      Serial.println();
    }

    delay(5000);
  }
}
#else
void ip5328_dump() {
}
#endif
