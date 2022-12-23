#ifndef ALARM_MANAGER_H_
#define ALARM_MANAGER_H_

#include "stdint.h"

void alarm_manager_init();
void alarm_manager_refresh_levels();
uint16_t alarm_manager_getlevel(uint8_t level);
void alarm_manager_on_main_loop();
bool alarm_manager_is_alarm();
void alarm_manager_mute_alarm();

void isrcall_alarm_manager_onimpulse();
void isrcall_alarm_manager_onresetmillis();

#endif /* ALARM_MANAGER_H_ */
