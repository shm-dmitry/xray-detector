#include "display.h"

#include "clock.h"

#define DISPLAY_DC_PIN      A3
#define DISPLAY_RESET_PIN   A2
#define DISPLAY_ENABLE_PIN  A1
#define TFT_DC 9
#define TFT_CS 10

//Adafruit_ST7735 display_tft = Adafruit_ST7735(-1, DISPLAY_DC_PIN, DISPLAY_RESET_PIN);
Adafruit_ILI9341 * display_tft = NULL;

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);

  display_on();
}

void display_on() {
  if (display_tft) {
    return;
  }

  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  clock_delay(10); // await for a power up

  display_tft = new Adafruit_ILI9341(-1, DISPLAY_DC_PIN, DISPLAY_RESET_PIN);
    //displat_tft.initR(INITR_BLACKTAB);
  display_tft->begin();
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
  delete display_tft;
  display_tft = NULL;
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == HIGH && display_tft;
}

Adafruit_SPITFT * display_get_object() {
  return display_tft;
}
