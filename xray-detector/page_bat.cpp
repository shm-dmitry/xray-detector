#include "page_bat.h"

#include "display.h"
#include "charger_control.h"

bool page_bat_refresh() {
  Adafruit_SPITFT * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(3);

  t_charger_data data = { 0 };
  if (charger_control_get_data(data)) {
    tft->print(data.bat_voltage_x10 / 10);
    tft->print(".");
    tft->print(data.bat_voltage_x10 % 10);
    tft->print("V");
  } else {
    tft->print("....");
  }
  
  return true;
}
