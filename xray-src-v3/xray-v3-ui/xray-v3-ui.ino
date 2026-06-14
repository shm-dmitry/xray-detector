#include "display.h"

#include "Arduino.h"

#include "display.h"
#include "uart_ctrl.h"
#include "rad_api.h"
#include "bat_state.h"
#include "user_input.h"
#include "ext_clock_ctrl.h"

void setup() {
  while (!ext_clock_ctrl_init()) {
    delay(100);
  }

  display_init();
  rad_api_init();
  bat_state_init();
  user_input_init();
  uart_ctrl_init();
}

void loop() {
  uart_ctrl_on_main_loop();
}
