#include "uart_ctrl.h"

#include "Arduino.h"
#include <string.h>

#include "uv_gen.h"

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
  pinMode(PIN_PB5, INPUT);
  uart_ctrl_changestate(digitalRead(PIN_PB5) == HIGH);
  attachInterrupt(digitalPinToInterrupt(PIN_PB5), isrcall__uart_ctrl_onuartchanged, CHANGE);
}

void uart_ctrl_changestate(bool enable) {
  if (enable && !uart_ctrl_enabled) {
    Serial.begin(9600);
    Serial.println("UART Initialized");
    uart_ctrl_enabled = true;
    uart_buffer_idx = 0;
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
    if (strncmp(uart_buffer, "uv set ", 7) == 0) {
        uint8_t parsed = (uint8_t)atoi(uart_buffer+ 7);

        uv_gen_start_on(parsed);
        Serial.print(F("Started with "));
        Serial.println(parsed);
    } 
    else if (strcmp(uart_buffer, "uv stop") == 0) {
        uv_gen_stop();
        Serial.println(F("Stopped"));
    } 
    else if (strcmp(uart_buffer, "uv start") == 0) {
        uv_gen_start();
        Serial.println(F("Started"));
    } 
    else if (strcmp(uart_buffer, "uv get") == 0) {
        t_uv_status_struct status;
        memset(&status, 0, sizeof(t_uv_status_struct));
        uv_get_getstatus(&status);

        Serial.print(F("UV status : "));
        Serial.println(status.status, HEX);  

        Serial.print(F("UV duty : "));
        Serial.println(status.duty);  

        Serial.print(F("UV :fb_counter "));
        Serial.println(status.fb_counter);  

        Serial.print(F("UV gen_counter : "));
        Serial.println(status.gen_counter);  

        Serial.print(F("UV fb_int_calls : "));
        Serial.println(status.fb_int_calls);  
    } 
    else if (strcmp(uart_buffer, "uv search") == 0) {
        uv_gen_start_search();
        Serial.println(F("S started"));
    } 
    else {
        Serial.println(F("Bad command"));
    }
}
