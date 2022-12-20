#include "page_flash.h"

#include "svf_control.h"
#include "display.h"

uint8_t flash_page_percent = 0;
bool flash_page_changing = false;

bool flash_page_refresh() {
  Adafruit_SPITFT * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(10);
  if (flash_page_percent == 0) {
    tft->print("OFF");
  } else {
    tft->print(flash_page_percent);
    tft->print(" %");
  }

  if (flash_page_changing) {
    tft->drawRoundRect(5, 50, 100, 10, 2, DISPLAY_WHITE);
    tft->fillRoundRect(5, 50, flash_page_percent, 10, 2, DISPLAY_WHITE);
  }

  return true;
}

bool flash_page_on_left() {
  if (!flash_page_changing) {
    return false;
  }

  if (flash_page_percent > 10) {
    flash_page_percent-=10;
  } else {
    flash_page_percent = 0;
  }

  svf_control_flash(flash_page_percent);
  
  return true;
}

bool flash_page_on_right() {
  if (!flash_page_changing) {
    return false;
  }

  if (flash_page_percent < 100) {
    flash_page_percent+=10;
    if (flash_page_percent > 100) {
      flash_page_percent = 100;
    }
  }

  svf_control_flash(flash_page_percent);

  return true;
}

bool flash_page_on_click() {
  flash_page_changing = !flash_page_changing;
  return true;
}
