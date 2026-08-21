#include "uart_ctrl.h"

#include "rad_api.h"
#include "alert_manager.h"

#include "Arduino.h"

#define MAX_COMMAND_LENGTH 32

char uart_buffer[MAX_COMMAND_LENGTH + 1];
uint8_t uart_buffer_idx = 0;

volatile bool uart_ctrl_pin_changed = false;
bool uart_ctrl_enabled = false;

void uart_ctrl_changestate(boolean enable);
void uart_process_command();

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
      char incomingChar = Serial.read();

      if (incomingChar == '\n' || incomingChar == '\r') { // Конец строки
        if (uart_buffer_idx > 0) { 
          uart_buffer[uart_buffer_idx] = '\0'; 
          uart_buffer_idx = 0; 

          uart_process_command();
        }
      } else if (uart_buffer_idx < MAX_COMMAND_LENGTH - 1) {
        uart_buffer[uart_buffer_idx++] = incomingChar;
      }
    }
  }
}

void uart_process_command() {
  if (strcmp(uart_buffer, "ra uvst") == 0) {
    Serial.println(rad_api_get_uv_status());
  } 
  else if (strcmp(uart_buffer, "ra rst") == 0) {
    Serial.println(rad_api_get_rad_status());
  }
  else if (strcmp(uart_buffer, "ra val") == 0) {
    uint8_t decimals = rad_api_get_decimals();

    uint32_t sbt9 = 0;
    uint32_t sbm20 = 0;
    bool ok = rad_api_get_current(&sbt9, &sbm20);

    Serial.print("Decimals = ");
    Serial.println(decimals);
    if (ok) {
      Serial.print("SBT9 = ");
      Serial.println(sbt9);
      Serial.print("SBM20 = ");
      Serial.println(sbt9);
    } else {
      Serial.println("Get last error");
    }
  }
  else if (strncmp(uart_buffer, "am ", 3) == 0) {
    uint8_t decimals = rad_api_get_decimals();

    char *endptr;

    uint32_t level1 = strtoul(uart_buffer + 3, &endptr, 10);
    uint32_t level2 = 0;
    if (endptr != (uart_buffer + 3) && *endptr == ' ') {
      char *start2 = endptr + 1;
      level2 = strtoul(start2, &endptr, 10);
      if (endptr != start2) {
        alert_manager_set_level_value(level1, level2);
      }
    }

    alert_manager_get_level_value(&level1, &level2);
    Serial.print("LV1 = ");
    Serial.print(level1 / decimals);
    Serial.print(".");
    Serial.print((uint8_t)(level1 % decimals));
    Serial.print("; LV1 = ");
    Serial.print(level2 / decimals);
    Serial.print(".");
    Serial.println((uint8_t)(level2 % decimals));
  }
  else if (strncmp(uart_buffer, "am", 3) == 0) {
    uint8_t decimals = rad_api_get_decimals();
    uint32_t level1 = 0;
    uint32_t level2 = 0;
    alert_manager_get_level_value(&level1, &level2);
    Serial.print("LV1 = ");
    Serial.print(level1 / decimals);
    Serial.print(".");
    Serial.print((uint8_t)(level1 % decimals));
    Serial.print("; LV1 = ");
    Serial.print(level2 / decimals);
    Serial.print(".");
    Serial.println((uint8_t)(level2 % decimals));
  }
}

bool uart_ctrl_can_go_sleep() {
  return !uart_ctrl_pin_changed && !uart_ctrl_enabled;
}

void uart_ctrl_print_buffer(uint32_t * buffer, uint8_t size) {
  if (uart_ctrl_enabled) {
    for (uint8_t i = 0; i<size; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(", ");
    }
    Serial.println();
  } 
}

void uart_ctrl_prints(const char * text) {
  if (uart_ctrl_enabled) {
    Serial.println(text);
  }
}

