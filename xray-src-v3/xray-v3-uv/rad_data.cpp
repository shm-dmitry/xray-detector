#include "Arduino.h"

#include "rad_data.h"
#include "rad_control.h"
#include "string.h"

typedef struct rad_data_struct_t {
  uint32_t prev_end_period_counter;
  uint32_t half_period_counter;
  uint32_t total_counter;

  uint8_t prev_end_period_seconds;
  uint8_t half_period_seconds;
  uint8_t total_seconds;
} rad_data_struct_t;

#define RAD_HISTORY_BUFFER_SIZE (20)
#define RAD_HISTORY_BUFFER_ELEMENT_SECONDS (30)
#define RAD_HISTORY_HALF_BUFFER_ELEMENT_SECONDS (30/2)
#define RAD_HISTORY_FIXED_POINT (10)

#define SBM20_PULSES_PER_MR (29)
#define SBT9_PULSES_PER_MR (15)

#define RAD_CALC(counter, seconds, pmr) (((uint32_t)1000 * (counter) * RAD_HISTORY_FIXED_POINT) / ((seconds) * (pmr)))

#define MIN_COUNTER_TO_CALC (15)
#define MAX_COUNTER_VALUE (0xFFFFFFFF / 1000 / RAD_HISTORY_FIXED_POINT)
#define RAD_HISTORY_ALLERT      (500 * RAD_HISTORY_FIXED_POINT)

uint32_t rad_history_sbm20[RAD_HISTORY_BUFFER_SIZE];
uint32_t rad_history_sbt9[RAD_HISTORY_BUFFER_SIZE];

volatile uint8_t rad_data_seconds = 0;
volatile bool rad_data_alert = false;

volatile rad_data_struct_t rad_data_sbt9;
volatile rad_data_struct_t rad_data_sbm20;
volatile bool rad_data_updated = false;
volatile bool rad_data_reset_period = false;
volatile bool rad_data_initialized = false;
volatile uint8_t rad_data_mask = 0;

void isrcall__rad_data_recalc(uint32_t value, volatile rad_data_struct_t * data, bool reset);
bool recalculate_buffer(const rad_data_struct_t * data, uint32_t* buffer, uint8_t pmr);

void rad_data_init() {
  pinMode(PIN_PB4, OUTPUT);
  digitalWrite(PIN_PB4, HIGH);

  memset(rad_history_sbm20, 0, sizeof(uint32_t) * RAD_HISTORY_BUFFER_SIZE);
  memset(rad_history_sbt9, 0, sizeof(uint32_t) * RAD_HISTORY_BUFFER_SIZE);
  memset((void *)&rad_data_sbt9, 0, sizeof(rad_data_struct_t));
  memset((void *)&rad_data_sbm20, 0, sizeof(rad_data_struct_t));
}

void isrcall__rad_data_on_one_second() {
  rad_data_seconds++;

  uint32_t sbt9 = 0;
  uint32_t sbm20 = 0;

  bool reset = rad_data_seconds >= RAD_HISTORY_BUFFER_ELEMENT_SECONDS;

  isrcall__rad_control_get_data(&sbt9, &sbm20, reset);

  isrcall__rad_data_recalc(sbt9, &rad_data_sbt9, reset);
  isrcall__rad_data_recalc(sbm20, &rad_data_sbm20, reset);

  rad_data_updated = true;
  rad_data_reset_period = rad_data_reset_period || reset;

  if (reset) {
    rad_data_seconds = 0;
  }
}

void isrcall__rad_data_recalc(uint32_t value, volatile rad_data_struct_t * data, bool reset) {
  data->total_seconds = rad_data_seconds;
  data->total_counter = value;
  if (rad_data_seconds <= RAD_HISTORY_HALF_BUFFER_ELEMENT_SECONDS) {
    data->half_period_counter = value;
    data->half_period_seconds = rad_data_seconds;
  }

  if (reset) {
    data->prev_end_period_counter = data->total_counter - data->half_period_counter;
    data->prev_end_period_seconds = data->total_seconds - data->half_period_seconds;
  }
}

uint8_t isrcall__rad_data_get_mask() {
  if (rad_data_alert) {
    return rad_data_mask | RAD_DATA_MASK__ALERT;
  }

  return rad_data_mask;
}

void isrcall__rad_data_set_mask(uint8_t mask) {
  rad_data_mask = mask & RAD_DATA_MASK;
}

void rad_data_on_main_loop() {
  if (rad_data_updated) {
    rad_data_struct_t sbt9;
    rad_data_struct_t sbm20;

    noInterrupts();
    memcpy(&sbt9, (const void *)&rad_data_sbt9, sizeof(rad_data_struct_t));
    memcpy(&sbm20, (const void *)&rad_data_sbm20, sizeof(rad_data_struct_t));

    bool reset_period = rad_data_reset_period;

    rad_data_updated = false;
    rad_data_reset_period = false;
    interrupts();

    bool sbt9_updated = recalculate_buffer(&sbt9, rad_history_sbt9, SBT9_PULSES_PER_MR);
    bool sbm20_updated = recalculate_buffer(&sbm20, rad_history_sbm20, SBM20_PULSES_PER_MR);

    bool wasAlert = rad_data_alert;
    rad_data_alert = 
        (sbt9_updated && rad_history_sbt9[0] == 0) || 
        rad_history_sbt9[0] >= RAD_HISTORY_ALLERT ||
        (sbm20_updated && rad_history_sbm20[0] == 0) ||
        rad_history_sbm20[0] >= RAD_HISTORY_ALLERT;

    if (rad_data_alert && !wasAlert) {
      digitalWrite(PIN_PB4, LOW);
    } else if (!rad_data_alert && wasAlert) {
      digitalWrite(PIN_PB4, HIGH);
    }

    uint8_t next_mask = sbt9_updated ? RAD_DATA_MASK__SBT9_CURRENT_UPDATED : 0;
    if (sbm20_updated) {
      next_mask |= RAD_DATA_MASK__SBM20_CURRENT_UPDATED;
    }

    if (reset_period) {
      next_mask |= (RAD_DATA_MASK__SBM20_HIST_UPDATED | RAD_DATA_MASK__SBT9_HIST_UPDATED);

      for (uint8_t i = RAD_HISTORY_BUFFER_SIZE - 1; i >= 1; i--) {
        rad_history_sbt9[i] = rad_history_sbt9[i - 1];
        rad_history_sbm20[i] = rad_history_sbm20[i - 1];
      }

      if (!rad_data_initialized) {
        if (rad_history_sbt9[0] > 0 || rad_history_sbm20[0] > 0) {
          rad_data_initialized = true;
        }
      }
    }

    rad_data_mask = next_mask;
  }
}

bool recalculate_buffer(const rad_data_struct_t * data, uint32_t* buffer, uint8_t pmr) {
  bool success = true;
  if (data->total_counter > MAX_COUNTER_VALUE) {
    buffer[0] = 0xFFFFFFFF;
  } else if (data->total_counter > MIN_COUNTER_TO_CALC) {
    buffer[0] = RAD_CALC(data->total_counter, data->total_seconds, pmr);
  } else if (data->prev_end_period_counter > MAX_COUNTER_VALUE) {
    buffer[0] = 0xFFFFFFFF;
  } else if (data->prev_end_period_counter + data->total_counter > MIN_COUNTER_TO_CALC && data->prev_end_period_seconds + data->total_seconds > 0) {
    buffer[0] = RAD_CALC(data->prev_end_period_counter + data->total_counter, data->prev_end_period_seconds + data->total_seconds, pmr);
  } else if (rad_data_initialized) {
    buffer[0] = 0;
  } else {
    success = false;
  }

  return success;
}

void rad_data_getlast(uint32_t * sbt9, uint32_t * sbm20, uint8_t * decimals) {
  *sbt9 = rad_history_sbt9[0];
  *sbm20 = rad_history_sbm20[0];

  *decimals = RAD_HISTORY_FIXED_POINT;
}

bool isrcall__rad_data_is_alert() {
  return rad_data_alert;
}

uint8_t isrcall__rad_data_decimals() {
  return RAD_HISTORY_FIXED_POINT;
}

uint8_t isrcall__rad_data_buffer_pointer(bool sbt9, uint8_t index, uint8_t maxelements, uint32_t ** ptr) {
  if (index >= RAD_HISTORY_BUFFER_SIZE) {
    *ptr = NULL;
    return 0;
  }

  if (index + maxelements > RAD_HISTORY_BUFFER_SIZE) {
    maxelements = RAD_HISTORY_BUFFER_SIZE - index;
  }
  if (maxelements == 0 || maxelements > RAD_HISTORY_BUFFER_SIZE) {
    *ptr = NULL;
    return 0;
  }

  *ptr = (sbt9 ? rad_history_sbt9 : rad_history_sbm20) + index;
  return maxelements;
}

bool isrcall__rad_data_allow_sleep() {
  return !rad_data_alert && !rad_data_updated && !rad_data_reset_period;
}
