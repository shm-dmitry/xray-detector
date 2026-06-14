#include "i2c_ctrl.h"

#include "Arduino.h"
#include <Wire.h>
#include "stdint.h"

#define I2C_SLAVE_ADDRESS 0x12

volatile bool i2c_ctrl_pin_changed = false;
bool i2c_ctrl_enabled = false;

void i2c_ctrl_changestate(boolean enable);
void isrcall__i2c_ctrl_onuartchanged();
void isrcall__i2c_onreceive(int bytes);
void isrcall__i2c_onrequest();

void i2c_ctrl_init() {
  pinMode(PIN_PA5, INPUT);
  i2c_ctrl_changestate(digitalRead(PIN_PA5) == HIGH);
  attachInterrupt(digitalPinToInterrupt(PIN_PA5), isrcall__i2c_ctrl_onuartchanged, CHANGE);
}

void isrcall__i2c_ctrl_onuartchanged() {
  i2c_ctrl_pin_changed = true;
}

void i2c_ctrl_changestate(bool enable) {
  if (enable && !i2c_ctrl_enabled) {
    Wire.swap(1); // переключаемся на PB0/1
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onReceive(isrcall__i2c_onreceive);
    Wire.onRequest(isrcall__i2c_onrequest);

    i2c_ctrl_enabled = true;
  } else if (!enable && i2c_ctrl_enabled) {
    Wire.end();
    i2c_ctrl_enabled = false;
    pinMode(PIN_PB0, PIN_INPUT_DISABLE);
    pinMode(PIN_PB1, PIN_INPUT_DISABLE);
  }
}


void i2c_ctrl_on_main_loop() {
  if (i2c_ctrl_pin_changed) {
    i2c_ctrl_pin_changed = false;
    i2c_ctrl_changestate(digitalRead(PIN_PA5) == HIGH);
  }
}

void isrcall__i2c_onreceive(int) {

}

void isrcall__i2c_onrequest() {

}

