#include "rad_control.h"

#include "Arduino.h"
#include "uv_gen.h"

#define RADCONTROL_PULSES_BEFORE_RECHARGE (500)

volatile uint32_t radcontrol_sbm20_counter = 0;
volatile uint32_t radcontrol_sbt9_counter = 0;

volatile uint32_t radcontrol_pulses_before_recharge = 0;

void isrcall__rad_control_sbm20_onimpuls();
void isrcall__rad_control_sbt9_onimpuls();

void rad_control_init() {
  pinMode(PIN_PB4, OUTPUT);
  digitalWrite(PIN_PB4, HIGH);

  pinMode(PIN_PA2, INPUT);
  pinMode(PIN_PA3, INPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_PA2), isrcall__rad_control_sbm20_onimpuls, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_PA3), isrcall__rad_control_sbt9_onimpuls, FALLING);
}

void isrcall__rad_control_sbm20_onimpuls() {
  radcontrol_sbm20_counter++;
  radcontrol_pulses_before_recharge++;
}

void isrcall__rad_control_sbt9_onimpuls() {
  radcontrol_sbt9_counter++;
  radcontrol_pulses_before_recharge++;
}

void isrcall__rad_control_on_one_second() {
}

void rad_control_on_main_loop() {
  noInterrupts();
  if (radcontrol_pulses_before_recharge > RADCONTROL_PULSES_BEFORE_RECHARGE) {
    radcontrol_pulses_before_recharge = 0;
    interrupts();

    uv_gen_start();
  } else {
    interrupts();    
  }
}
