#include "rad_control.h"

#include "Arduino.h"
#include "uv_gen.h"
#include "rad_data.h"
#include "uart_ctrl.h"
#include "timer_ctrl.h"

#define RADCONTROL_DROP_PER_SECOND_MV         (40)
#define RADCONTROL_DROP_PER_SBM20_PULSE_MV    (50)
#define RADCONTROL_DROP_PER_SBT9_PULSE_MV     (80)
#define RADCONTROL_VOLTAGE_DROP_THRESHOLD_MV  (20000)

#define RADCONTROL_DROP_ALLOW_INCREMENT       (0xFFFF - 100)

#define RADCONTROL_DROP_TO_RECHARGE_WHEN_STOPPED (RADCONTROL_VOLTAGE_DROP_THRESHOLD_MV - 5 * RADCONTROL_DROP_PER_SECOND_MV)

volatile uint32_t radcontrol_sbm20_counter = 0;
volatile uint32_t radcontrol_sbt9_counter = 0;

volatile uint16_t radcontrol_accumulated_drop = 0;

#if DEBUG__PRINT_IMPL
uint32_t radcontrol_last_sbt9 = 0;
uint32_t radcontrol_last_sbm20 = 0;
#endif

#if NRAD_COMMAND_ACTIVE
uint32_t radcontrol_total_sbt9 = 0;
uint32_t radcontrol_total_sbm20 = 0;
#endif

void isrcall__rad_control_sbm20_onimpuls();
void isrcall__rad_control_sbt9_onimpuls();

void rad_control_init() {
  pinMode(PIN_PA2, INPUT);
  pinMode(PIN_PA3, INPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_PA2), isrcall__rad_control_sbm20_onimpuls, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_PA3), isrcall__rad_control_sbt9_onimpuls, FALLING);
}

void isrcall__rad_control_sbm20_onimpuls() {
  if (radcontrol_sbm20_counter != 0xFFFFFFFF) {
    radcontrol_sbm20_counter++;
  }
  if (radcontrol_accumulated_drop < RADCONTROL_DROP_ALLOW_INCREMENT) {
    radcontrol_accumulated_drop += RADCONTROL_DROP_PER_SBM20_PULSE_MV;
  }
#if NRAD_COMMAND_ACTIVE
  radcontrol_total_sbm20++;
#endif
}

void isrcall__rad_control_sbt9_onimpuls() {
  if (radcontrol_sbt9_counter != 0xFFFFFFFF) {
    radcontrol_sbt9_counter++;
  }
  if (radcontrol_accumulated_drop < RADCONTROL_DROP_ALLOW_INCREMENT) {
    radcontrol_accumulated_drop += RADCONTROL_DROP_PER_SBT9_PULSE_MV;
  }
#if NRAD_COMMAND_ACTIVE
  radcontrol_total_sbt9++;
#endif
}

void isrcall__rad_control_on_one_second() {
  if (radcontrol_accumulated_drop < RADCONTROL_DROP_ALLOW_INCREMENT) {
    radcontrol_accumulated_drop += RADCONTROL_DROP_PER_SECOND_MV;
  }
}

void isrcall__rad_control_get_data(uint32_t * sbt9, uint32_t * sbm20, bool reset) {
  *sbt9 = radcontrol_sbt9_counter;
  *sbm20 = radcontrol_sbm20_counter;

  if (reset) {
    radcontrol_sbt9_counter = 0;
    radcontrol_sbm20_counter = 0;
  }
}

#if NRAD_COMMAND_ACTIVE
void rad_control_get_total_counters(uint32_t * sbt9, uint32_t * sbm20, bool reset) {
  noInterrupts();
  *sbt9 = radcontrol_total_sbt9;
  *sbm20 = radcontrol_total_sbm20;

  if (reset) {
    radcontrol_total_sbt9 = 0;
    radcontrol_total_sbm20 = 0;
  }
  interrupts();
}
#endif

void rad_control_on_main_loop() {
  bool startGen = false;

  noInterrupts();
  if (timerctrl_is_alive() && radcontrol_accumulated_drop >= RADCONTROL_VOLTAGE_DROP_THRESHOLD_MV) {
    radcontrol_accumulated_drop = 0;
    startGen = true;
  }
  interrupts();

  bool alert = isrcall__rad_data_is_alert();

  if (startGen || alert) {
    if (!uv_gen_recharge(alert)) {
      noInterrupts();
      radcontrol_accumulated_drop = RADCONTROL_DROP_TO_RECHARGE_WHEN_STOPPED;
      interrupts();
    }
  }

  if (!alert && isrcall__uv_get_getstatus() == UV_GEN_STATUS_CHARGING_CONTINUUS) {
    uv_gen_recharge(false);
  }

#if DEBUG__PRINT_IMPL
  noInterrupts();
  uint32_t dumpsbt9 = (radcontrol_sbt9_counter > radcontrol_last_sbt9) ? radcontrol_sbt9_counter - radcontrol_last_sbt9 : 0;
  uint32_t dumpsbm20 = (radcontrol_sbm20_counter > radcontrol_last_sbm20) ? radcontrol_sbm20_counter - radcontrol_last_sbm20 : 0;

  radcontrol_last_sbt9 = radcontrol_sbt9_counter;
  radcontrol_last_sbm20 = radcontrol_sbm20_counter;
  interrupts();

  if (dumpsbt9 > 0 || dumpsbm20 > 0) {
    uart_ctrl_print_impl(dumpsbt9, dumpsbm20);
  }
#endif
}
