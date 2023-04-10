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

void display_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  display_tft->fillRect(x, y, w, h, color);
}

void display_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  display_tft->drawRect(x, y, w, h, color);
}

void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color) {
  display_tft->drawBitmap(x, y, bitmap, w, h, color);
}

void display_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  display_tft->drawLine(x0, y0, x1, y1, color);
}

void display_set_cursor(uint8_t x, uint8_t y) {
  display_tft->setCursor(x, y);
}

void display_set_textcolor(uint16_t color) {
  display_tft->setTextColor(color);
}

void display_set_textsize(uint8_t size) {
  display_tft->setTextSize(size);
}

void display_prints(const char * text) {
  display_tft->print(text);
}

void display_println(const char * text) {
  display_tft->println(text);
}

void display_print8(uint8_t value) {
  display_tft->print(value);
}

void display_print16(uint16_t value) {
  display_tft->print(value);
}

void display_print32(uint32_t value) {
  display_tft->print(value);
}

uint8_t display_get_cursor_x() {
  return display_tft->getCursorX();
}

uint8_t display_get_cursor_y() {
  return display_tft->getCursorY();
}
