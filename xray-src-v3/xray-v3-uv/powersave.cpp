#include "core_devices.h"
#include "powersave.h"

#include "Arduino.h"
#include <avr/sleep.h>

#include "uv_gen.h"
#include "uart_ctrl.h"
#include "i2c_ctrl.h"
#include "rad_data.h"

#define POWERSAVE_ENABLED false

void powersave_on_main_loop() {
  #if POWERSAVE_ENABLED
  noInterrupts();

  uint8_t uvstatus = isrcall__uv_get_getstatus();
  if (uvstatus != UV_GEN_STATUS_IDLE && uvstatus != UV_GEN_STATUS_NOFB) {
    interrupts();
    return;
  }

  if (isrcall__uart_is_active()) {
    interrupts();
    return;
  }

  if (isrcall__i2c_is_active()) {
    interrupts();
    return;
  }
\
  if (!isrcall__rad_data_allow_sleep()) {
    interrupts();
    return;
  }

  uint8_t vref = VREF.CTRLB;
  VREF.CTRLB = 0;

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

  uint8_t ac0 = AC0.CTRLA;
  uint8_t ac1 = AC1.CTRLA;
  AC0.CTRLA = 0;
  AC1.CTRLA = 0;

  uint8_t adc0 = ADC0.CTRLA;
  ADC0.CTRLA = 0;

  SLPCTRL.CTRLA = SLPCTRL_SMODE_PDOWN_gc;

  interrupts();

  sleep_enable();
  sleep_cpu();
  sleep_disable();

  ADC0.CTRLA = adc0;
  AC1.CTRLA = ac1;
  AC0.CTRLA = ac0;
  TWI0.SCTRLA = i2cS;
  TWI0.MCTRLA = i2cM;
  SPI0.CTRLA = spi;
  USART0.CTRLB = uartB;
  USART0.CTRLA = uartA;
  VREF.CTRLB = vref;
  #endif
}
