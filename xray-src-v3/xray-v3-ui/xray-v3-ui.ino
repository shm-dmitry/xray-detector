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

  display_on();
  display_set_cursor(20, 20);
  display_set_textcolor(DISPLAY_RED);
  display_set_textsize(3);
  display_prints("I'm working!");
}

void loop() {
  uart_ctrl_on_main_loop();
}
