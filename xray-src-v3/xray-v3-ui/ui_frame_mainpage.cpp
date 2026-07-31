#include "display.h"
#include "ui_frame_mainpage.h"

#include "rad_api.h"

uint32_t ui_frame_mainpage_last_value_sbt9;
uint32_t ui_frame_mainpage_last_value_sbm20;
uint8_t ui_frame_mainpage_decimals;
bool ui_frame_mainpage_lasterror;

void ui_frame_mainpage_printrad(uint32_t value, bool sbt9, bool hide);
void ui_frame_mainpage_printerr(bool sbt9, bool hide);

void ui_frame_mainpage_refresh() {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    ui_frame_mainpage_decimals = rad_api_get_decimals();
  }

  if (ui_frame_mainpage_decimals > 0 && ui_frame_mainpage_decimals < RAD_DATA_MASK__NODATA) {
    uint32_t sbt9 = 0;
    uint32_t sbm20 = 0;

    if (rad_api_get_current(&sbt9, &sbm20)) {
      if (ui_frame_mainpage_lasterror) {
        ui_frame_mainpage_printerr(true, true);
        ui_frame_mainpage_printerr(false, true);
        ui_frame_mainpage_printrad(sbt9, true, false);
        ui_frame_mainpage_printrad(sbm20, false, false);

        ui_frame_mainpage_lasterror = false;
        ui_frame_mainpage_last_value_sbt9 = sbt9;
        ui_frame_mainpage_last_value_sbm20 = sbm20;
      } else {
        if (sbt9 != ui_frame_mainpage_last_value_sbt9) {
          ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbt9, true, true);
          ui_frame_mainpage_printrad(sbt9, true, false);

          ui_frame_mainpage_last_value_sbt9 = sbt9;
        } 
        
         if (sbm20 != ui_frame_mainpage_last_value_sbm20) {
          ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbm20, false, true);
          ui_frame_mainpage_printrad(sbm20, false, false);

          ui_frame_mainpage_last_value_sbm20 = sbm20;
        }
      }
    } else {
      if (!ui_frame_mainpage_lasterror) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbt9, true, true);
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbm20, false, true);

        ui_frame_mainpage_printerr(true, false);
        ui_frame_mainpage_printerr(false, false);

        ui_frame_mainpage_lasterror = true;
      }
    }
  } else {
    ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbt9, true, true);
    ui_frame_mainpage_printrad(ui_frame_mainpage_last_value_sbm20, false, true);

    ui_frame_mainpage_printerr(true, false);
    ui_frame_mainpage_printerr(false, false);

    ui_frame_mainpage_lasterror = true;
  }
}

void ui_frame_mainpage_init() {
  ui_frame_mainpage_last_value_sbt9 = 0;
  ui_frame_mainpage_last_value_sbm20 = 0;
  ui_frame_mainpage_decimals = 0;
}

void ui_frame_mainpage_printrad(uint32_t value, bool sbt9, bool hide) {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    return;
  }

  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  if (sbt9) {
    display_set_cursor(20, 100);
  } else {
    display_set_cursor(220, 100);
  }

  display_set_textsize(3);

  display_print16(value / ui_frame_mainpage_decimals);
  display_prints(".");
  display_print8(value % ui_frame_mainpage_decimals);
  display_prints(" uR/H");
}

void ui_frame_mainpage_printerr(bool sbt9, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_RED);
  }

  if (sbt9) {
    display_set_cursor(20, 100);
  } else {
    display_set_cursor(220, 100);
  }

  display_set_textsize(3);

  display_prints("ERR");
}


