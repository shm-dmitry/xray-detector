#include "uv_control.h"
#include "rad_control.h"
#include "clock.h"
#include "svf_control.h"
#include "display.h"
#include "userinput.h"
#include "gui_manager.h"
#include "powersave.h"
#include "eeprom_control.h"
#include "alarm_manager.h"
#include "charger_control.h"
//#include "ip5328p_dump.h"

#define UV_DEBUG_MODE false

void setup() {
//  ip5328_dump();
  
  Serial.begin(9600);

#if UV_DEBUG_MODE
  uv_control_init();
  uv_control_update_pwm(105000, 7);
  uv_control_enable_pwm();
  while(true);
#endif

  clock_init();
  svf_control_init();
  eeprom_control_init();
  clock_init();
  rad_control_init();
  uv_control_init();
  svf_control_init();
  userinput_init();
  display_init(); 
  gui_manager_init();
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
  gui_manager_on_main_loop();

  clock_delay(5);
}
