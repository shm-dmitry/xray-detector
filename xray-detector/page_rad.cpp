#include "page_rad.h"

#include "rad_control.h"
#include "display.h"

void RadPage::refresh() {
  uint32_t rad = rad_control_dose();

  Adafruit_ST7735 * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(ST77XX_BLACK);
  tft->setTextColor(ST77XX_WHITE);
  tft->setTextSize(10);

  if (rad == 0) {
    tft->print("NO DATA");
  } else {
    tft->print(rad);
    tft->print(" mkR/hour");
  }
}
