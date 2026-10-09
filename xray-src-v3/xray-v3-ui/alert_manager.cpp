#include "alert_manager.h"

#include "rad_api.h"

#define ALERT_MANAGER_ALERT_SBT9_ZERO  false
#define ALERT_MANAGER_ALERT_SBM20_ZERO true

#define ALERT_MANAGER_LEVEL1 (100)
#define ALERT_MANAGER_LEVEL2 (500)

uint32_t alert_manager_level1;
uint32_t alert_manager_level2;

void alert_manager_on_wakeup() {
  uint8_t decimals = rad_api_get_decimals();
  if (decimals > 0) {
    if (alert_manager_level1 == 0) {
      alert_manager_level1 = decimals * ALERT_MANAGER_LEVEL1;
    }
    if (alert_manager_level2 == 0) {
      alert_manager_level2 = decimals * ALERT_MANAGER_LEVEL2;
    }

    uint8_t mask = 0;
    #if ALERT_MANAGER_ALERT_SBT9_ZERO
    mask |= RAD_ALERT_MASK__SBT9_ALERTZER0_EN;
    #endif
    #if ALERT_MANAGER_ALERT_SBM20_ZERO
    mask |= RAD_ALERT_MASK__SBM20_ALERTZER0_EN;
    #endif

    if (alert_manager_level1 > 0) {
      mask |= (RAD_ALERT_MASK__SBM20_ALERTHIGH_EN | RAD_ALERT_MASK__SBT9_ALERTHIGH_EN);
    }

    rad_api_alert_setmask(mask);
    rad_api_alert_set_high_value(true, alert_manager_level1);
    rad_api_alert_set_high_value(false, alert_manager_level1);
  }
}

uint8_t alert_manager_getlevel(uint32_t value) {
  if (value >= alert_manager_level2) {
    return 2;
  } else if (value >= alert_manager_level1) {
    return 1;
  } else {
    return 0;
  }
}

void alert_manager_set_level_value(uint32_t level1, uint32_t level2) {
  uint8_t decimals = rad_api_get_decimals();

  if (decimals > 0) {
    alert_manager_level1 = level1 * decimals;
    alert_manager_level2 = level2 * decimals;

    alert_manager_on_wakeup();
  }
}

void alert_manager_get_level_value(uint32_t * level1, uint32_t * level2) {
  *level1 = alert_manager_level1;
  *level2 = alert_manager_level2;
}

