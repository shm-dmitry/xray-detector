#include "display.h"

#include "clock.h"

#define DISPLAY_DC_PIN      A3
#define DISPLAY_ENABLE_PIN  A1

#if DISPLAY_SIMUL
Adafruit_ILI9341 * display_tft = NULL;
#else
Adafruit_ST7735 * display_tft = NULL;
#endif

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  clock_delay(1000);

  display_on();
}

void display_on() {
  if (display_tft) {
    return;
  }

  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
  clock_delay(100); // await for a power up

#if DISPLAY_SIMUL
  display_tft = new Adafruit_ILI9341(-1, DISPLAY_DC_PIN, -1);
  display_tft->begin();
#else
  display_tft = new Adafruit_ST7735(-1, DISPLAY_DC_PIN, -1);
  display_tft->setSPISpeed(1000000);
  display_tft->initR(INITR_BLACKTAB);
  display_tft->setRotation(3);
#endif

  display_tft->fillRect(0, 0, 160, 128, DISPLAY_BLACK);
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  digitalWrite(DISPLAY_DC_PIN, LOW);
  clock_delay(100);
  delete display_tft;
  display_tft = NULL;
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == LOW && display_tft != NULL;
}

Adafruit_SPITFT * display_get_object() {
  return display_tft;
}
