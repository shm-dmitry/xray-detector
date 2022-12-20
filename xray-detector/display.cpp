#include "display.h"

#include "clock.h"

#define DISPLAY_DC_PIN      A3
#define DISPLAY_RESET_PIN   A2
#define DISPLAY_ENABLE_PIN  A1
#define TFT_DC 9
#define TFT_CS 10

//Adafruit_ST7735 displat_tft = Adafruit_ST7735(-1, DISPLAY_DC_PIN, DISPLAY_RESET_PIN);
Adafruit_ILI9341 displat_tft = Adafruit_ILI9341(-1, A3, A2);

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);

  display_on();
}
unsigned long testText();
void display_on() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  clock_delay(10); // await for a power up
  //displat_tft.initR(INITR_BLACKTAB);
  displat_tft.begin();
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == HIGH;
}

Adafruit_SPITFT * display_get_object() {
  return &displat_tft;
}
