#include "charger_control.h"

#include "Arduino.h"
#include <Wire.h>
#include "powersave.h"
#include "clock.h"

#define CHARGER_BUTTON_PIN A6
#define CHARGER_I2C_EN_PIN A7

#define CHARGER_I2C_ADDR_W 0xEA
#define CHARGER_I2C_ADDR_R (CHARGER_I2C_ADDR_W + 1)

// stage1 - charge controller needs some time to enter I2C mode, at least 500 msec
#define CHARGER_I2C_START_DELAY_1     600
// stage2 - capacity on I2C level converter will charge over resister, at least 1 msec
#define CHARGER_I2C_START_DELAY_2     3

volatile uint32_t charger_control_enable_i2c_time = 0;

void charger_control_init() {
  pinMode(CHARGER_BUTTON_PIN, OUTPUT);
  digitalWrite(CHARGER_BUTTON_PIN, LOW);
  pinMode(CHARGER_I2C_EN_PIN, OUTPUT);
  digitalWrite(CHARGER_I2C_EN_PIN, LOW);

  pinMode(CHARGER_INT_PIN, INPUT);

  if (digitalRead(CHARGER_INT_PIN) == HIGH) {
    charger_control_enable_i2c_time = clock_millis();
  }

  Wire.begin();
}

void isrcall_charger_control_onusbint() {
  charger_control_enable_i2c_time = clock_millis(true);
  isrcall_powersave_onwakeup();
}

void charger_control_on_main_loop() {
  if (charger_control_enable_i2c_time == 0 && 
      digitalRead(CHARGER_INT_PIN) == HIGH && 
      digitalRead(CHARGER_I2C_EN_PIN) == LOW) {
    // we awakes after sleep mode, but charger still works
    charger_control_enable_i2c_time = clock_millis();
  }
  
  if (charger_control_enable_i2c_time > 0) {
    if (digitalRead(CHARGER_INT_PIN) == LOW) {
      // charger powered-off during init-i2c process
      charger_control_enable_i2c_time = 0;
      digitalWrite(CHARGER_I2C_EN_PIN, LOW);
    } else {
      uint32_t now = clock_millis();
      bool ovf = false;
      uint32_t untill = clock_calc_delay(charger_control_enable_i2c_time, CHARGER_I2C_START_DELAY_1, ovf);
      
      if ((ovf && (now < charger_control_enable_i2c_time && now >= untill)) ||
          (!ovf && (now >= untill))) {
          if (digitalRead(CHARGER_I2C_EN_PIN) == LOW) {
            // start stage2
            digitalWrite(CHARGER_I2C_EN_PIN, HIGH);
            clock_delay(CHARGER_I2C_START_DELAY_2);

            // all delays done, we are ready to get data from charger
            charger_control_enable_i2c_time = 0; 
          }
      }
    }
  }
}

bool charger_control_get_data(t_charger_data & data) {
  if (digitalRead(CHARGER_INT_PIN) == LOW) {
    // wake up charge controller
    digitalWrite(CHARGER_BUTTON_PIN, HIGH);
    clock_delay(100);
    digitalWrite(CHARGER_BUTTON_PIN, LOW);

    for (uint8_t i = 0; i<5; i++) {
      // await some time to make sure charger wakes up
      if (digitalRead(CHARGER_INT_PIN) == HIGH) {
        break;
      }

      clock_delay(10);
    }
    
    return false;
  }

  if (charger_control_enable_i2c_time > 0 || digitalRead(CHARGER_I2C_EN_PIN) == LOW) {
    // await for a I2C startups
    return false;
  }

  // TODO: I2C ready, reading..
  data.bat_voltage_x10 = 42;
  return true;
}

void charger_control_enter_sleep_mode() {
  charger_control_enable_i2c_time = 0;
  digitalWrite(CHARGER_I2C_EN_PIN, LOW);
}
