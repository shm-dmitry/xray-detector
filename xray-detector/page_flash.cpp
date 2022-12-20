#include "page_flash.h"

#include "svf_control.h"
#include "display.h"

void FlashPage::refresh() {
  Adafruit_ST7735 * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(ST77XX_BLACK);
  tft->setTextColor(ST77XX_WHITE);
  tft->setTextSize(10);
  if (percent == 0) {
    tft->print("OFF");
  } else {
    tft->print(percent);
    tft->print(" %");
  }

  if (changing) {
    tft->drawRoundRect(5, 50, 100, 10, 2, ST77XX_WHITE);
    tft->fillRoundRect(5, 50, percent, 10, 2, ST77XX_WHITE);
  }
}

bool FlashPage::on_left() {
  if (!changing) {
    return false;
  }

  if (percent > 0) {
    percent--;
  }

  svf_control_flash(percent);
  
  return true;
}

bool FlashPage::on_right() {
  if (!changing) {
    return false;
  }

  if (percent < 100) {
    percent++;
  }

  svf_control_flash(percent);

  return true;
}

void FlashPage::on_click() {
  changing = !changing;
}
