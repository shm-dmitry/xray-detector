#include "display.h"

#include "clock.h"

#define DISPLAY_DC_PIN      PC3
#define DISPLAY_RESET_PIN   PC2
#define DISPLAY_ENABLE_PIN  PC1

Adafruit_ST7735 displat_tft = Adafruit_ST7735(-1, DISPLAY_DC_PIN, DISPLAY_RESET_PIN);

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);

  display_on();
}

void display_on() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  clock_delay(10); // await for a power up
  displat_tft.initR(INITR_BLACKTAB);
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == HIGH;
}

Adafruit_ST7735 * display_get_object() {
  return &displat_tft;
}
