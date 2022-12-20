#include "uv_control.h"
#include "rad_control.h"
#include "clock.h"
#include "svf_control.h"
#include "display.h"
#include "userinput.h"
#include "pages_control.h"

void setup() {
  Serial.begin(9600);

  clock_init();
  rad_control_init();
  uv_control_init();
  svf_control_init();
  userinput_init();
  display_init(); 
  pages_control_init();
  Serial.println("Init done");
}

void loop() {
  uv_control_manualpwm_correct();
  svf_control_on_main_loop();
  rad_control_on_main_loop();
  pages_on_main_loop();

  clock_delay(5);
}
