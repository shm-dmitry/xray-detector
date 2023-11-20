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
#include "rad_history.h"
#include "ip5328p_dump.h"
#include "config.h"
#include "graph.h"

#define UV_DEBUG_MODE false

void setup() {
//  ip5328_dump();
//  return;
  
#if SYSTEM_SERIAL_ENABLED
  Serial.begin(SYSTEM_SERIAL_FREQ);
#endif

#if UV_DEBUG_MODE
  uv_control_init();
  uv_control_update_pwm(105000, 7);
  uv_control_enable_pwm();
  while(true);
#endif

  clock_init();
  svf_control_init();
  eeprom_control_init();
  rad_control_init();
  svf_control_init();
  userinput_init();
  display_init(); 
  gui_manager_init();
  alarm_manager_init();
  charger_control_init();
  uv_control_init();
  rad_history_init();

#if SYSTEM_SERIAL_ENABLED
  Serial.println("Init done");
#endif
}

void loop() {
  alarm_manager_on_main_loop();
  
  if (!powersave_on_main_loop()) {
    rad_history_on_main_loop();
    graph_refresh_from_history();
    return;
  }
  
  charger_control_on_main_loop();
  svf_control_on_main_loop();
  rad_history_on_main_loop();
  gui_manager_on_main_loop();
}
