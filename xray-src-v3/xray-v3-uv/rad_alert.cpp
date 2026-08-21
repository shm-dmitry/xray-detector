#include "rad_alert.h"

#include "rad_data.h"

#define RAD_ALERT_DEFAULT_VALUE (200)

volatile uint8_t rad_alert_mask;
volatile uint32_t rad_alert_sbt9;
volatile uint32_t rad_alert_sbm20;

bool rad_alert_hasnonzero_sbt9;
bool rad_alert_hasnonzero_sbm20;

void rad_alert_init() {
  rad_alert_mask = RAD_ALERT_MASK__SBT9_ALERTHIGH_EN | RAD_ALERT_MASK__SBM20_ALERTZER0_EN | RAD_ALERT_MASK__SBM20_ALERTHIGH_EN | RAD_ALERT_MASK__SBT9_ALERTZER0_EN;
  rad_alert_sbt9 = isrcall__rad_data_decimals() * RAD_ALERT_DEFAULT_VALUE;
  rad_alert_sbm20 = isrcall__rad_data_decimals() * RAD_ALERT_DEFAULT_VALUE;
  rad_alert_hasnonzero_sbt9 = false;
  rad_alert_hasnonzero_sbm20 = false;
}

void isrcall__rad_alert_setmask(uint8_t mask) {
  rad_alert_mask = mask & RAD_ALERT_MASKS;
}

uint8_t isrcall__rad_alert_getmask() {
  return rad_alert_mask;
}

void isrcall__rad_alert_set_high_value(bool sbt9, uint32_t value) {
  if (sbt9) {
    rad_alert_sbt9 = value;
  } else {
    rad_alert_sbm20 = value;
  }
}

uint32_t isrcall__rad_alert_get_high_value(bool sbt9) {
  return sbt9 ? rad_alert_sbt9 : rad_alert_sbm20;
}

bool rad_alert_test(uint32_t sbt9, uint32_t sbm20) {
  if (sbt9 == 0 && rad_alert_hasnonzero_sbt9 && (rad_alert_mask & RAD_ALERT_MASK__SBT9_ALERTZER0_EN)) {
    return true;
  }
  if (sbm20 == 0 && rad_alert_hasnonzero_sbm20 && (rad_alert_mask & RAD_ALERT_MASK__SBM20_ALERTZER0_EN)) {
    return true;
  }

  if (sbt9 > 0) {
    rad_alert_hasnonzero_sbt9 = true;

    if (sbt9 > rad_alert_sbt9 && (rad_alert_mask & RAD_ALERT_MASK__SBT9_ALERTHIGH_EN)) {
      return true;
    }
  }

  if (sbm20 > 0) {
    rad_alert_hasnonzero_sbm20 = true;

    if (sbm20 > rad_alert_sbm20 && (rad_alert_mask & RAD_ALERT_MASK__SBM20_ALERTHIGH_EN)) {
      return true;
    }
  }

  return false;
}
