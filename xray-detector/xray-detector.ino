#include "uv_control.h"
#include "rad_control.h"
#include "clock.h"
#include "svf_control.h"
#include "display.h"
#include "userinput.h"
#include "pages_control.h"
#include "powersave.h"
#include "eeprom_control.h"
#include "alarm_manager.h"
#include "charger_control.h"

void setup() {
  Serial.begin(9600);

  eeprom_control_init();
  clock_init();
  rad_control_init();
  uv_control_init();
  svf_control_init();
  userinput_init();
  display_init(); 
  pages_control_init();
  powersave_init();
  alarm_manager_init();
  charger_control_init();
  
  Serial.println("Init done");
}

void loop() {
  alarm_manager_on_main_loop();
  
  if (!powersave_on_main_loop()) {
    return;
  }
  
  charger_control_on_main_loop();
  svf_control_on_main_loop();
  pages_on_main_loop();

  clock_delay(5);
}
