#include "gui_page_rad.h"
#include "rad_control.h"
#include "display.h"
#include "alarm_manager.h"
#include "clock.h"

uint32_t rad_page_dose = 0xFFFFFFFF;

void gui_rad_page_draw_dose(Adafruit_SPITFT * tft, uint32_t dose);

bool gui_rad_page_refresh(uint8_t data) {
  Adafruit_SPITFT * tft = display_get_object();
  if (tft == NULL) {
    return;
  }

  if (data) {
    rad_page_dose = 0xFFFFFFFF;
  }

  uint32_t dose = rad_control_dose();
  if (rad_page_dose != dose) {
    gui_rad_page_draw_dose(tft, dose);
    rad_page_dose = dose;
  }

  return true;
}

void gui_rad_page_draw_dose_text(Adafruit_SPITFT * tft, uint32_t dose) {
  if (dose < 1000) {
    tft->setTextSize(3);
    tft->print(dose);
    tft->setTextSize(2);
    tft->setCursor(tft->getCursorX(), tft->getCursorY() + 8);
    tft->print(" uR/h");
  } else if (dose < 1000000) {
    tft->setTextSize(3);
    tft->print(dose / 1000);
    tft->print(".");
    tft->print((dose % 1000) / 100);
    tft->setCursor(tft->getCursorX(), tft->getCursorY() + 8);
    tft->setTextSize(2);
    tft->print(" mR/h");
  } else {
    tft->setTextSize(3);
    tft->print(dose / 1000000);
    tft->print(".");
    tft->print((dose % 1000000) / 100000);
    tft->setCursor(tft->getCursorX(), tft->getCursorY() + 8);
    tft->setTextSize(2);
    tft->print(" R/h");
  }
}

void gui_rad_page_draw_dose(Adafruit_SPITFT * tft, uint32_t dose) {
  uint8_t level = alarm_manager_dose2level(dose);
  tft->setCursor(10, 20);
  tft->setTextColor(DISPLAY_BLACK);
  gui_rad_page_draw_dose_text(tft, rad_page_dose);
  tft->setCursor(10, 20);
  tft->setTextColor(level == 0 ? DISPLAY_GREEN : (level == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
  gui_rad_page_draw_dose_text(tft, dose);
}
