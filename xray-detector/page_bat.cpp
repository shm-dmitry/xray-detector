#include "page_bat.h"

#include "display.h"
#include "charger_control.h"
#include "clock.h"

#define PAGE_BAT_REFRESH_EVERY 1000

uint32_t page_bat_last_refresh = 0;

bool page_bat_refresh() {
  page_bat_last_refresh = clock_millis();
  
  Adafruit_SPITFT * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(2);

  t_charger_data data = { 0 };
  if (charger_control_get_data(data)) {
    tft->print("Bat: ");
    tft->print(data.bat_voltage_x100 / 100);
    tft->print(".");
    tft->print(data.bat_voltage_x100 % 100);
    tft->println("V");

    tft->print("I: ");
    tft->print(data.bat_current_x10 / 10);
    tft->print(".");
    tft->print(data.bat_current_x10 % 10);
    tft->println("A");

    tft->print("InUse: ");
    if (data.i2c_in_use) {
      tft->println("I2C");
    } else {
      tft->println("ADC");
    }
  } else {
    tft->print("....");
  }
  
  return true;
}

bool page_bat_need_refresh() {
  return clock_is_elapsed(page_bat_last_refresh, PAGE_BAT_REFRESH_EVERY);
}
