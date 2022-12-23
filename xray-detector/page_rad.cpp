#include "page_rad.h"

#include "rad_control.h"
#include "display.h"
#include "alarm_manager.h"

uint32_t page_rad_last = 0;

bool page_rad_refresh() {
  page_rad_last = rad_control_dose();

  Adafruit_SPITFT * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(3);

  if (page_rad_last == 0) {
    tft->print("NO DATA");
  } else {
    tft->print(page_rad_last);
    tft->print(" mkR/hour");
  }

  return true;
}

bool page_rad_onclick() {
  alarm_manager_mute_alarm();
}

bool page_rad_need_refresh() {
  return page_rad_last != rad_control_dose();
}
