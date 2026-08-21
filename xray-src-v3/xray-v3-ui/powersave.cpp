#include "powersave.h"

#include "ext_clock_ctrl.h"
#include "user_input.h"
#include "uart_ctrl.h"
#include "rad_api.h"
#include "ui_manager.h"
#include "i2c_ctrl.h"
#include "bat_state.h"
#include "alert_manager.h"
#include "Arduino.h"
#include <avr/sleep.h>

#define POWERSAVE_ALLOW_SLEEP true
#define POWERSAVE_NEXT_GO_SLEEP (20*1000)

void powersave_wakeup();
void powersave_prepare_sleep();
bool powersave_can_sleep();
void powersave_execute_sleep();

uint32_t powersave_next_go_sleep = 0;
bool powersave_is_sleep = true;

void powersave_on_main_loop() {
  if (powersave_can_sleep()) {
    if (!powersave_is_sleep) {
      powersave_prepare_sleep();
      powersave_is_sleep = true;
    }

    while(true) {
      powersave_execute_sleep();
      if (!powersave_can_sleep()) {
        powersave_wakeup();
        powersave_is_sleep = false;
        break;
      }
    }
  } else {
    if (powersave_is_sleep) {
      powersave_wakeup();
      powersave_is_sleep = false;
    }
  }
}

bool powersave_can_sleep() {
  if (!user_input_can_sleep()) {
    return false;
  }

  if (!uart_ctrl_can_go_sleep()) {
    return false;
  }

  if (!rad_api_can_go_sleep()) {
    return false;
  }

  if (!bat_state_can_sleep()) {
    return false;
  }

  if (powersave_next_go_sleep > ext_clock_millis()) {
    return false;
  }

  return POWERSAVE_ALLOW_SLEEP;
}

void powersave_wakeup() {
  ext_clock_wake_up();
  i2c_ctrl_on_wake_up();
  rad_api_wakeup();
  ui_manager_wakeup();
  bat_state_on_wake_up();
  alert_manager_on_wakeup();

  powersave_next_go_sleep = ext_clock_millis() + POWERSAVE_NEXT_GO_SLEEP;
}

void powersave_prepare_sleep() {
  i2c_ctrl_on_go_sleep();
  rad_api_go_sleep();
  ui_manager_go_sleep();
  bat_state_on_go_sleep();
  ext_clock_go_sleep();
}

void powersave_execute_sleep() {
#if  POWERSAVE_ALLOW_SLEEP
  noInterrupts();

  uint8_t uartA = USART0.CTRLA;
  uint8_t uartB = USART0.CTRLB;

  USART0.CTRLA = 0;
  USART0.CTRLB = 0;

  uint8_t spi = SPI0.CTRLA;
  SPI0.CTRLA = 0;

  uint8_t i2cM = TWI0.MCTRLA;
  uint8_t i2cS = TWI0.SCTRLA;
  TWI0.MCTRLA = 0;
  TWI0.SCTRLA = 0;

  SLPCTRL.CTRLA = SLPCTRL_SMODE_PDOWN_gc;

  interrupts();

  sleep_enable();
  sleep_cpu();
  sleep_disable();

  TWI0.SCTRLA = i2cS;
  TWI0.MCTRLA = i2cM;
  SPI0.CTRLA = spi;
  USART0.CTRLB = uartB;
  USART0.CTRLA = uartA;
#endif
}
