#pragma once

#include "stdint.h"

#define RAD_ALERT_MASK__SBT9_ALERTZER0_EN       (0b00000001)
#define RAD_ALERT_MASK__SBT9_ALERTHIGH_EN       (0b00000010)
#define RAD_ALERT_MASK__SBM20_ALERTZER0_EN      (0b00000100)
#define RAD_ALERT_MASK__SBM20_ALERTHIGH_EN      (0b00001000)

#define RAD_ALERT_MASKS                         (0b00001111)

void rad_alert_init();

void isrcall__rad_alert_setmask(uint8_t mask);
uint8_t isrcall__rad_alert_getmask();
void isrcall__rad_alert_set_high_value(bool sbt9, uint32_t value);
uint32_t isrcall__rad_alert_get_high_value(bool sbt9);

bool rad_alert_test(uint32_t sbt9, uint32_t sbm20);
