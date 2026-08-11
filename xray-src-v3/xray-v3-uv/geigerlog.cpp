#include "geigerlog.h"

#if GEIGERLOG_ENABLED

#include "Arduino.h"
#include "uart_ctrl.h"
#include "rad_data.h"
#include <string.h>

#define GEIGERLOG_PRINT_EACH_SECONDS 5

volatile uint8_t geigerlog_timer = 0;
bool geigerlog_enabled = true;

void isrcall__geigerlog_on_one_second() {
  if (geigerlog_timer < 0xFF) {
    geigerlog_timer++;
  }
}

void geigerlog_enable(bool enable) {
  geigerlog_enabled = enable;
}

void geigerlog_on_main_loop() {
  if (!geigerlog_enabled) {
    return;
  }

  bool print = false;
  noInterrupts();
  if (geigerlog_timer >= GEIGERLOG_PRINT_EACH_SECONDS) {
    geigerlog_timer = 0;
    print = true;
  }
  interrupts();

  if (print && isrcall__uart_is_active()) {
    uint32_t sbt9 = 0;
    uint32_t sbm20 = 0;

    rad_data_getcpm(&sbt9, &sbm20);

    uart_ctrl_print("CPM9: ");
    uart_ctrl_printn(sbt9);

    uart_ctrl_print(", CPM20: ");
    uart_ctrl_printn(sbm20);

    uint8_t decimals;
    rad_data_getlast(&sbt9, &sbm20, &decimals);

    if (isrcall__rad_data_is_alert()) {
      uart_ctrl_print(", ALERT: ON");
    } else {
      uart_ctrl_print(", ALERT: OFF");
    }

    uart_ctrl_print(", SBT9: ");
    uart_ctrl_printn(sbt9 / decimals);
    uart_ctrl_print(".");
    uart_ctrl_printn(sbt9 % decimals);
    uart_ctrl_print(" uR/H");

    uart_ctrl_print(", SBM20: ");
    uart_ctrl_printn(sbm20 / decimals);
    uart_ctrl_print(".");
    uart_ctrl_printn(sbm20 % decimals);
    uart_ctrl_println(" uR/H");
  }
}
#endif
