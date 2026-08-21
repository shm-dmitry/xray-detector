#include "rad_data.h"

#include "Arduino.h"

#include "rad_control.h"
#include "rad_alert.h"
#include "string.h"

typedef struct rad_data_cpm_t {
  volatile uint32_t * buffer;
  uint8_t buffer_size;
  uint8_t buffer_index;
  uint16_t tau;
} rad_data_cpm_t;

#define RAD_HISTORY_BUFFER_SIZE (20)
#define RAD_HISTORY_BUFFER_ELEMENT_SECONDS (30)
#define RAD_HISTORY_FIXED_POINT (10)

#define RAD_RINGBUFFER_SECONDS (5)

#define SBT9_ACCUMULATED_COUNTER_TO_CALC (30)
#define SBM20_ACCUMULATED_COUNTER_TO_CALC (40)

// 105 & 200 - from datasheet
#define SBM20_TUBES_COUNT (2)
#define SBM20_PULSES_PER_UR (63 * SBM20_TUBES_COUNT)
#define SBT9_PULSES_PER_UR (15)

#define SBT9_RINGBUFFER_SIZE (2 * 60 / RAD_RINGBUFFER_SECONDS)
#define SBT9_MIN_COUNTER_TO_CALC (15)
// TODO: check tau!
#define SBT9_CORRECTION_TAU (150)
#define SBM20_RINGBUFFER_SIZE (1 * 60 / RAD_RINGBUFFER_SECONDS)
#define SBM20_MIN_COUNTER_TO_CALC (20)
// TODO: check tau!
#define SBM20_CORRECTION_TAU (200 / SBM20_TUBES_COUNT)

#define MIN_N_UNUSE_CORRECTION (100)

// TODO: потом проверить формулу
#define RAD_CALC(counter, seconds, pur) (((counter) * 3600 * RAD_HISTORY_FIXED_POINT) / ((pur) * (seconds)))

#define MAX_COUNTER_VALUE       (0xFFFFFFFF / 3600 / RAD_HISTORY_FIXED_POINT)

uint32_t rad_history_sbm20[RAD_HISTORY_BUFFER_SIZE];
uint32_t rad_history_sbt9[RAD_HISTORY_BUFFER_SIZE];

volatile uint8_t rad_data_seconds = 0;
volatile uint8_t rad_buffer_seconds = 0;
volatile bool rad_data_alert = false;

volatile rad_data_cpm_t rad_data_sbt9;
volatile rad_data_cpm_t rad_data_sbm20;
volatile bool rad_data_updated = false;
volatile bool rad_data_reset_period = false;
volatile bool rad_data_initialized = false;
volatile uint8_t rad_data_mask = 0;

void isrcall__rad_data_recalc(uint32_t value, volatile rad_data_cpm_t * data, bool reset);
bool rad_data_recalculate_buffer(uint32_t accumulated, uint8_t seconds, uint32_t* buffer, uint8_t pmr, uint32_t min_counter);

void rad_data_init() {
  pinMode(PIN_PB4, OUTPUT);
  digitalWrite(PIN_PB4, HIGH);

  memset(rad_history_sbm20, 0, sizeof(uint32_t) * RAD_HISTORY_BUFFER_SIZE);
  memset(rad_history_sbt9, 0, sizeof(uint32_t) * RAD_HISTORY_BUFFER_SIZE);

  rad_data_sbt9.buffer = (uint32_t *) malloc(SBT9_RINGBUFFER_SIZE * sizeof(uint32_t));
  memset((void *)rad_data_sbt9.buffer, 0, SBT9_RINGBUFFER_SIZE * sizeof(uint32_t));
  rad_data_sbt9.buffer_size = SBT9_RINGBUFFER_SIZE;
  rad_data_sbt9.buffer_index = 0;
  rad_data_sbt9.tau = SBT9_CORRECTION_TAU;

  rad_data_sbm20.buffer = (uint32_t *) malloc(SBM20_RINGBUFFER_SIZE * sizeof(uint32_t));
  memset((void *)rad_data_sbm20.buffer, 0, SBM20_RINGBUFFER_SIZE * sizeof(uint32_t));
  rad_data_sbm20.buffer_size = SBM20_RINGBUFFER_SIZE;
  rad_data_sbm20.buffer_index = 0;
  rad_data_sbm20.tau = SBM20_CORRECTION_TAU;
}

void isrcall__rad_data_on_one_second() {
  rad_data_seconds++;
  rad_buffer_seconds++;

  uint16_t sbt9 = 0;
  uint16_t sbm20 = 0;

  bool reset = rad_buffer_seconds > RAD_RINGBUFFER_SECONDS;

  isrcall__rad_control_get_data(&sbt9, &sbm20, reset);

  isrcall__rad_data_recalc(sbt9, &rad_data_sbt9, reset);
  isrcall__rad_data_recalc(sbm20, &rad_data_sbm20, reset);

  rad_data_updated = true;
  if (rad_data_seconds >= RAD_HISTORY_BUFFER_ELEMENT_SECONDS) {
    rad_data_reset_period = true;
    rad_data_seconds = 0;
  }

  if (reset) {
    rad_buffer_seconds = 0;
  }
}

uint32_t isrcall__rad_data_correction(uint32_t value, uint8_t t_seconds, uint16_t tau_us) {
  if (t_seconds == 0 || value < MIN_N_UNUSE_CORRECTION * t_seconds) {
    return value;
  }

  uint32_t t_ms = t_seconds * 1000;
  uint32_t busy_ms = (value * tau_us) / 1000;
  if (busy_ms >= (t_ms * 8) / 10) {
    return 0xFFFFFFFF;
  }

  return (value * 1000) / (t_ms - busy_ms);
}

void isrcall__rad_data_recalc(uint32_t value, volatile rad_data_cpm_t * data, bool reset) {
  if (reset) {
    data->buffer_index++;
    if (data->buffer_index >= data->buffer_size) {
      data->buffer_index = 0;
    }
  }

  data->buffer[data->buffer_index] = isrcall__rad_data_correction(value, reset ? RAD_RINGBUFFER_SECONDS : rad_buffer_seconds, data->tau);
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

void isrcall__rad_data_extract_counter(volatile rad_data_cpm_t * data, uint32_t * counter, uint8_t * seconds, uint32_t limitcounter) {
  if (data->buffer[data->buffer_index] > 0) {
    *counter = data->buffer[data->buffer_index];
    *seconds = rad_data_seconds;
  }


  if (*counter >= MAX_COUNTER_VALUE) {
    *counter = MAX_COUNTER_VALUE;
    return;
  }

  if (data->buffer_index > 0) {
    for (int8_t i = data->buffer_index - 1; i>=0; i--) {
      *counter += data->buffer[i];
      *seconds += RAD_RINGBUFFER_SECONDS;

      if (*counter >= MAX_COUNTER_VALUE) {
        *counter = MAX_COUNTER_VALUE;
        return;
      }

      if (*counter >= limitcounter) {
        return;
      }
    }
  }

  for (int8_t i = data->buffer_size - 1; i > data->buffer_index; i--) {
    *counter += data->buffer[i];
    *seconds += RAD_RINGBUFFER_SECONDS;

    if (*counter >= MAX_COUNTER_VALUE) {
      *counter = MAX_COUNTER_VALUE;
      return;
    }

    if (*counter >= limitcounter) {
      return;
    }
  }
}

void rad_data_on_main_loop() {
  if (rad_data_updated) {
    uint32_t sbt9counter = 0;
    uint8_t sbt9seconds = 0;
    uint32_t sbm20counter = 0;
    uint8_t sbm20seconds = 0;

    noInterrupts();
    isrcall__rad_data_extract_counter(&rad_data_sbt9, &sbt9counter, &sbt9seconds, SBT9_ACCUMULATED_COUNTER_TO_CALC);
    isrcall__rad_data_extract_counter(&rad_data_sbm20, &sbm20counter, &sbm20seconds, SBM20_ACCUMULATED_COUNTER_TO_CALC);

    bool reset_period = rad_data_reset_period;

    rad_data_updated = false;
    rad_data_reset_period = false;
    interrupts();

    bool sbt9_updated = rad_data_recalculate_buffer(sbt9counter, sbt9seconds, rad_history_sbt9, SBT9_PULSES_PER_UR, SBT9_MIN_COUNTER_TO_CALC);
    bool sbm20_updated = rad_data_recalculate_buffer(sbm20counter, sbm20seconds, rad_history_sbm20, SBM20_PULSES_PER_UR, SBM20_MIN_COUNTER_TO_CALC);

    bool wasAlert = rad_data_alert;
    rad_data_alert = rad_alert_test(rad_history_sbt9[0], rad_history_sbm20[0]);

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

bool rad_data_recalculate_buffer(uint32_t accumulated, uint8_t seconds, uint32_t* buffer, uint8_t pmr, uint32_t min_counter) {
  bool success = true;
  if (accumulated > MAX_COUNTER_VALUE) {
    buffer[0] = 0xFFFFFFFF;
  } else if (accumulated >= min_counter) {
    if (seconds > 0) {
      buffer[0] = RAD_CALC(accumulated, seconds, pmr);
    } else {
      buffer[0] = 0;
    }
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
  if (ptr == NULL) {
    return 0;
  }

  if (index >= RAD_HISTORY_BUFFER_SIZE || maxelements == 0) {
    *ptr = NULL;
    return 0;
  }

  if (index + maxelements > RAD_HISTORY_BUFFER_SIZE) {
    maxelements = RAD_HISTORY_BUFFER_SIZE - index;
  }

  *ptr = (sbt9 ? rad_history_sbt9 : rad_history_sbm20) + index;
  return maxelements;
}

bool isrcall__rad_data_allow_sleep() {
  return !rad_data_alert && !rad_data_updated && !rad_data_reset_period;
}

void isrcall__calculate_cpm(volatile rad_data_cpm_t * data, uint32_t * counter) {
  uint8_t seconds = 0;

  noInterrupts();
  *counter = data->buffer[data->buffer_index];
  seconds = rad_buffer_seconds;

  if (*counter >= MAX_COUNTER_VALUE) {
    *counter = 0xFFFFFFFF;
    return;
  }

  if (data->buffer_index > 0) {
    for (int8_t i = data->buffer_index - 1; i >= 0; i--) {
      *counter += data->buffer[i];
      seconds += RAD_RINGBUFFER_SECONDS;

      if (*counter >= MAX_COUNTER_VALUE) {
        *counter = 0xFFFFFFFF;
        return;
      }

      if (seconds >= 60) {
        return;
      }
    }
  }

  for (int8_t i = data->buffer_size - 1; i> data->buffer_index; i--) {
    *counter += data->buffer[i];
    seconds += RAD_RINGBUFFER_SECONDS;

    if (*counter >= MAX_COUNTER_VALUE) {
      *counter = 0xFFFFFFFF;
      return;
    }

    if (seconds >= 60) {
      return;
    }
  }
  interrupts();
}

void rad_data_getcpm(uint32_t * sbt9, uint32_t * sbm20) {
  isrcall__calculate_cpm(&rad_data_sbt9, sbt9);
  isrcall__calculate_cpm(&rad_data_sbm20, sbm20);
}

uint8_t isrcall__rad_data_buffer_size() {
  return RAD_HISTORY_BUFFER_SIZE;
}
