#include "display.h"

#include "Arduino.h"

#include "uart_ctrl.h"
#include "rad_api.h"
#include "bat_state.h"
#include "user_input.h"
#include "ext_clock_ctrl.h"
#include "powersave.h"
#include "ui_manager.h"

void setup() {
  ext_clock_ctrl_init();

  ui_manager_init();
  rad_api_init();
  bat_state_init();
  user_input_init();
  uart_ctrl_init();
}

void loop() {
  uart_ctrl_on_main_loop();
  bat_state_on_main_loop();
  ui_manager_on_main_loop();

  powersave_on_main_loop();
}
