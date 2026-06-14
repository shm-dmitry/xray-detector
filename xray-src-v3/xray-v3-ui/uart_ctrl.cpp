#include "uart_ctrl.h"

#include "Arduino.h"

volatile bool uart_ctrl_pin_changed = false;
bool uart_ctrl_enabled = false;

void uart_ctrl_changestate(boolean enable);

void isrcall__uart_ctrl_onuartchanged() {
  uart_ctrl_pin_changed = true;
}

void uart_ctrl_init() {
  pinMode(PIN_PA6, INPUT);
  uart_ctrl_changestate(digitalRead(PIN_PA6) == HIGH);
  attachInterrupt(digitalPinToInterrupt(PIN_PA6), isrcall__uart_ctrl_onuartchanged, CHANGE);
}

void uart_ctrl_changestate(bool enable) {
  if (enable && !uart_ctrl_enabled) {
    Serial.begin(9600);
    uart_ctrl_enabled = true;
  } else if (!enable && uart_ctrl_enabled) {
    Serial.end();
    uart_ctrl_enabled = false;
    pinMode(PIN_PB2, PIN_INPUT_DISABLE);
    pinMode(PIN_PB3, PIN_INPUT_DISABLE);
  }
}

void uart_ctrl_on_main_loop() {
  if (uart_ctrl_pin_changed) {
    uart_ctrl_pin_changed = false;
    uart_ctrl_changestate(digitalRead(PIN_PA6) == HIGH);
  }

  if (uart_ctrl_enabled) {
    if (Serial.available() > 0) {
      while(Serial.available() > 0) {
        Serial.read();
      }
    }
  }
}
