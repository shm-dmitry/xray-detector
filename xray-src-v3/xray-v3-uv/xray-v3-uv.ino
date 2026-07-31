#include "uv_gen.h"
#include "uart_ctrl.h"
#include "timer_ctrl.h"
#include "rad_control.h"
#include "i2c_ctrl.h"
#include "rad_data.h"
#include "powersave.h"

void setup() {
  uv_gen_init();
  timerctrl_init();
  uart_ctrl_init();
  i2c_ctrl_init();
  rad_data_init();
  rad_control_init();
}

void loop() {
  rad_data_on_main_loop();
  uart_ctrl_on_main_loop();
  i2c_ctrl_on_main_loop();
  rad_control_on_main_loop();

  powersave_on_main_loop();
}
