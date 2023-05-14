#ifndef ALARM_MANAGER_H_
#define ALARM_MANAGER_H_

#include "stdint.h"

#define ALARM_MANAGER_NI_LEVEL 0xF0

#define ALARM_MANAGER_ONIMPULSE_VOICE_NONE 0x00
#define ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO 0x01
#define ALARM_MANAGER_ONIMPULSE_VOICE_VOICE 0x02
#define ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE 0x03

#define ALARM_MANAGER_ONIMPULSE_VOICE_MAXVALUE ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE

void alarm_manager_init();
void alarm_manager_refresh_levels();
uint8_t alarm_manager_getlevel(uint8_t level);
uint8_t alarm_manager_dose2level(uint32_t dose);
void alarm_manager_on_main_loop();
bool alarm_manager_is_alarm();
void alarm_manager_mute_alarm();

void isrcall_alarm_manager_onimpulse();
void isrcall_alarm_manager_onresetmillis();
void isrcall_alarm_manager_onminute();

#endif /* ALARM_MANAGER_H_ */
