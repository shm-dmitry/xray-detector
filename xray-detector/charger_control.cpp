#include "charger_control.h"

#include "Arduino.h"
#include <Wire.h>
#include "powersave.h"
#include "clock.h"

#define CHARGER_BUTTON_PIN A2

#define CHARGER_I2C_ADDR 0x75

#define CHARGER_I2C_REG_VOLTAGE_1 0x64
#define CHARGER_I2C_REG_VOLTAGE_2 0x65

#define CHARGER_I2C_REG_CURRENT_1 0x66
#define CHARGER_I2C_REG_CURRENT_2 0x67

// charge controller needs some time to enter I2C mode, at least 500 msec
#define CHARGER_I2C_START_DELAY     600
// From datasheet: "Two short press in 1s: turn off boost output, ..." This delay prevent double-click
#define CHARGER_I2C_WAKE_UP_DELAYS  5000
#define CHARGER_I2C_TIMEOUT         50

#define CHARGER_I2C_DELAYS_DONE     0x01

#define CHARGER_ADC_ION             (11 * 50 / 10)
#define CHARGER_CONTROL_ADC_TIMEOUT 10

#define CHARGER_CONTROL_MAXV_X100   420
#define CHARGER_CONTROL_MINV_X100   310

volatile uint32_t charger_control_enable_i2c_time = 0;
volatile uint8_t charger_adc_voltage_x50 = 0;
uint32_t charger_control_last_wake_up = 0;

bool charger_control_read_adc(t_charger_data & data);

ISR (ADC_vect) {
  uint16_t adch = ADCH - 1;
  if (adch == 0) {
    charger_adc_voltage_x50 = 0;
  } else {
    charger_adc_voltage_x50 = ((uint16_t)CHARGER_ADC_ION * 255) / adch;
  }
}

uint8_t charger_read_byte(uint8_t address) {
  Wire.beginTransmission(CHARGER_I2C_ADDR);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(CHARGER_I2C_ADDR, 1, false);

  uint32_t now = clock_millis();
  while (!Wire.available()) {
    if (clock_is_elapsed(now, CHARGER_I2C_TIMEOUT)) {
      Wire.endTransmission();
      return 0;
    }
  }
  
  uint8_t v = Wire.read();
  Wire.endTransmission();
  return v;
}

void charger_control_init() {
  pinMode(CHARGER_BUTTON_PIN, OUTPUT);
  digitalWrite(CHARGER_BUTTON_PIN, LOW);

  pinMode(CHARGER_INT_PIN, INPUT);

  if (digitalRead(CHARGER_INT_PIN) == HIGH) {
    charger_control_enable_i2c_time = clock_millis();
  }

  Wire.begin();
  Wire.setClock(200000);

  ADCSRA = 0; // disable ADC
  ADMUX = (0 << REFS1) | (1 << REFS0) | // reference: internal 1.1V
          (1 << ADLAR) | // ADC Left Adjust Result
          (0 << MUX0) | (1 << MUX1)  | (1 << MUX2)  | (1 << MUX3); // input: 1.1V (VBG)
}

void isrcall_charger_control_onusbint() {
  if (digitalRead(CHARGER_INT_PIN) == HIGH) {
    if (charger_control_enable_i2c_time == 0) {
      charger_control_enable_i2c_time = clock_millis(true);
    }

    isrcall_powersave_onwakeup();
  } else {
    charger_control_enable_i2c_time = 0;
  }
}

void charger_control_on_main_loop() {
  if (charger_control_enable_i2c_time == 0 && 
      digitalRead(CHARGER_INT_PIN) == HIGH) {
    // we awakes after sleep mode, but charger still works
    charger_control_enable_i2c_time = clock_millis();
  }
  
  if (charger_control_enable_i2c_time > 0 && charger_control_enable_i2c_time != CHARGER_I2C_DELAYS_DONE) {
    if (digitalRead(CHARGER_INT_PIN) == LOW) {
      // charger powered-off during init-i2c process
      charger_control_enable_i2c_time = 0;
    } else {
      if (clock_is_elapsed(charger_control_enable_i2c_time, CHARGER_I2C_START_DELAY)) {
          charger_control_enable_i2c_time = CHARGER_I2C_DELAYS_DONE;
      }
    }
  }
}

void charger_control_read_i2c(t_charger_data & data) {
  uint32_t value = charger_read_byte(CHARGER_I2C_REG_VOLTAGE_2) << 8;
  value += charger_read_byte(CHARGER_I2C_REG_VOLTAGE_1);
  if (value == 0xFFFF || value == 0x0000) { // problems with I2C, use ADC
    charger_control_read_adc(data);
    return;
  }

  value = (value * 26855) / 1000000;
  data.bat_voltage_x100 = value + 260;

  value = charger_read_byte(CHARGER_I2C_REG_CURRENT_2) << 8;
  value += charger_read_byte(CHARGER_I2C_REG_CURRENT_1);

  if (value > 0x7FFF) {
    value = 0xFFFF - value;
  }

  data.bat_current_x10 = (value * 127883) / 10000000;
  data.i2c_in_use = true;
}

bool charger_control_get_data(t_charger_data & data) {
  if (digitalRead(CHARGER_INT_PIN) == LOW) {
    if (charger_control_last_wake_up != 0 && 
        !clock_is_elapsed(charger_control_last_wake_up, CHARGER_I2C_WAKE_UP_DELAYS)) {
      // dont spam controller via wake-up commands
      return charger_control_read_adc(data);
    }

    charger_control_last_wake_up = clock_millis();

#if CHARGER_CONTROL_ALLOW_WAKE_UP    
    // wake up charge controller
    digitalWrite(CHARGER_BUTTON_PIN, HIGH);
    clock_delay(100);
    digitalWrite(CHARGER_BUTTON_PIN, LOW);
#endif

    for (uint8_t i = 0; i<5; i++) {
      // await some time to make sure charger wakes up
      if (digitalRead(CHARGER_INT_PIN) == HIGH) {
        break;
      }

      clock_delay(10);
    }
    
    return charger_control_read_adc(data);
  }

  if (charger_control_enable_i2c_time > 0 && charger_control_enable_i2c_time != CHARGER_I2C_DELAYS_DONE) {
    // await for a I2C startups
    return charger_control_read_adc(data);
  }

  charger_control_read_i2c(data);

  return true;
}

bool charger_control_read_adc(t_charger_data & data) {
  ADCSRA =  (1 << ADEN) | // enable ADC
            (0 << ADSC) | // dont start now
            (1 << ADIE) | // enable interrupt
            (1 << ADPS2)|(1 << ADPS1)|(0 << ADPS0); // prescaler 64

  uint16_t result_x50 = 0;
  for (uint8_t i = 0; i<64; i++) { // 64 (==2^6) measurements to get average result.
    uint32_t now = clock_millis();
    charger_adc_voltage_x50 = 0xFF;
    ADCSRA |= (1 << ADSC);
    while (charger_adc_voltage_x50 == 0xFF) {
      if (clock_is_elapsed(now, CHARGER_CONTROL_ADC_TIMEOUT)) {
        return false;
      }
    }
    result_x50 += charger_adc_voltage_x50;
  }

  ADCSRA = 0; // Finish. Disable ADC.

  // >>6 -> div 64 to calc avg(x50), <<2 -> mult 2 to x50 -> x100. total >>5
  data.bat_voltage_x100 = (result_x50 >> 5);
  data.i2c_in_use = false;

  return data.bat_voltage_x100 > 0;
}

void charger_control_enter_sleep_mode() {
  charger_control_enable_i2c_time = 0;
}

uint8_t charger_control_get_voltage_pc() {
  t_charger_data data = { 0 };
  if (charger_control_read_adc(data)) {
    if (data.bat_voltage_x100 >= CHARGER_CONTROL_MAXV_X100) {
      return 100;
    } else if (data.bat_voltage_x100 <= CHARGER_CONTROL_MINV_X100) {
      return 0;
    }

    return ((data.bat_voltage_x100 - (uint16_t)CHARGER_CONTROL_MINV_X100) * 100) / ((uint16_t)CHARGER_CONTROL_MAXV_X100 - (uint16_t)CHARGER_CONTROL_MINV_X100);
  } else {
    return 0x00;
  }
}

