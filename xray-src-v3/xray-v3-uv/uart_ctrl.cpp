#include "uart_ctrl.h"

#include "Arduino.h"

#include "uv_gen.h"

volatile bool uart_ctrl_pin_changed = false;
bool uart_ctrl_enabled = false;

void uart_ctrl_changestate(boolean enable);

void isrcall__uart_ctrl_onuartchanged() {
  uart_ctrl_pin_changed = true;
}

void uart_ctrl_init() {
  pinMode(PIN_PB5, INPUT);
  uart_ctrl_changestate(digitalRead(PIN_PB5) == HIGH);
  attachInterrupt(digitalPinToInterrupt(PIN_PB5), isrcall__uart_ctrl_onuartchanged, CHANGE);
}

void uart_ctrl_changestate(bool enable) {
  if (enable && !uart_ctrl_enabled) {
    Serial.begin(9600);
    Serial.println("UART Initialized");
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
    uart_ctrl_changestate(digitalRead(PIN_PB5) == HIGH);
  }

  if (uart_ctrl_enabled) {
    if (Serial.available() > 0) {
      String s = Serial.readString();
      s.trim();
      if (s.startsWith("uv set ")) {
        String val = s.substring(7);
        val.trim();
        uint8_t parsed = atoi(val.c_str());

        uv_gen_start_on(parsed);
        Serial.print("Started with ");
        Serial.println(parsed);
      } else if (s == "uv stop") {
        uv_gen_stop();
        Serial.println("Stopped");
      } else if (s == "uv start") {
        uv_gen_start();
        Serial.println("Started");
      } else if (s == "uv get") {
        uint8_t v= uv_get_getstatus();
        Serial.print("UV status : ");
        Serial.println(v, HEX);  
      } else if (s == "uv reg") {
        Serial.print("TCA0.SPLIT.HPER=");
        Serial.println(TCA0.SPLIT.HPER, HEX);
        Serial.print("TCA0.SPLIT.HCMP1=");
        Serial.println(TCA0.SPLIT.HCMP1 ,HEX);
        Serial.print("TCA0.SPLIT.CTRLB=");
        Serial.println(TCA0.SPLIT.CTRLB ,HEX);
        Serial.print("TCA0.SPLIT.CTRLA=");
        Serial.println(TCA0.SPLIT.CTRLA ,HEX);
        Serial.print("CLKCTRL.MCLKCTRLB=");
        Serial.println(CLKCTRL.MCLKCTRLB, HEX);
        Serial.print("CLKCTRL.MCLKCTRLA=");
        Serial.println(CLKCTRL.MCLKCTRLA, HEX);
      }
    }
  }
}
