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
  Serial.println("display power down");

  display_on();
}

void display_on() {
  if (display_tft) {
    return;
  }

  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
  clock_delay(100); // await for a power up
  Serial.println("display power up");

#if DISPLAY_SIMUL
  display_tft = new Adafruit_ILI9341(-1, DISPLAY_DC_PIN, -1);
  display_tft->begin();
#else
  display_tft = new Adafruit_ST7735(-1, DISPLAY_DC_PIN, -1);
  display_tft->initR(INITR_BLACKTAB);
  display_tft->setRotation(3);
#endif
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  delete display_tft;
  display_tft = NULL;
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == LOW && display_tft;
}

Adafruit_SPITFT * display_get_object() {
  return display_tft;
}
