#include "display.h"

#include "Arduino.h"
#include "clock.h"
#include "config.h"

#if DISPLAY_TYOE_ILI9341_ADAFRUIT
#include <Adafruit_ILI9341.h>
#elif DISPLAY_TYPE_ST7335_ADAFRUIT
#include <Adafruit_ST7735.h>
#elif DISPLAY_TYPE_ILI9341_CUSTOM
#include "display_ili9341.h"
#elif DISPLAY_TYPE_ST7335_CUSTOM
#include "display_st7735.h"
#endif

#define DISPLAY_DC_PIN      A3
#define DISPLAY_ENABLE_PIN  6

#if DISPLAY_TYOE_ILI9341_ADAFRUIT
Adafruit_ILI9341 * display_tft = NULL;
#elif DISPLAY_TYPE_ST7335_ADAFRUIT
Adafruit_ST7735 * display_tft = NULL;
#endif

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  clock_delay(1000);

  display_on();
}

void display_on() {
#if SYSTEM_SERIAL_ENABLED
  Serial.println("display_on");
#endif

  if (display_is_on()) {
    return;
  }

  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
  clock_delay(30); // await for a power up

#if DISPLAY_TYOE_ILI9341_ADAFRUIT
  display_tft = new Adafruit_ILI9341(-1, DISPLAY_DC_PIN, -1);
  display_tft->begin();
#elif DISPLAY_TYPE_ST7335_ADAFRUIT
  display_tft = new Adafruit_ST7735(-1, DISPLAY_DC_PIN, -1);
  display_tft->initR(INITR_BLACKTAB);
  display_tft->setRotation(3);
#elif DISPLAY_TYPE_ILI9341_CUSTOM
  display_ili9341_init(DISPLAY_DC_PIN);
#elif DISPLAY_TYPE_ST7335_CUSTOM
  display_st7735_init(DISPLAY_DC_PIN);
#endif

  display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_BLACK);
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  digitalWrite(DISPLAY_DC_PIN, LOW);
  clock_delay(100);
#if DISPLAY_TYOE_ILI9341_ADAFRUIT or DISPLAY_TYPE_ST7335_ADAFRUIT
  delete display_tft;
  display_tft = NULL;
#endif
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == LOW;
}

#if DISPLAY_TYOE_ILI9341_ADAFRUIT or DISPLAY_TYPE_ST7335_ADAFRUIT
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_tft->fillRect(x, y, w, h, color);
}

void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_tft->drawRect(x, y, w, h, color);
}

void display_draw_bitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color) {
  display_tft->drawBitmap(x, y, bitmap, w, h, color);
}

void display_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  display_tft->drawLine(x0, y0, x1, y1, color);
}

void display_set_cursor(uint16_t x, uint16_t y) {
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

uint16_t display_get_cursor_x() {
  return display_tft->getCursorX();
}

uint16_t display_get_cursor_y() {
  return display_tft->getCursorY();
}
#elif DISPLAY_TYPE_ILI9341_CUSTOM or DISPLAY_TYPE_ST7335_CUSTOM
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_spi_fill_rect(x, y, w, h, color);
}

void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_spi_draw_rect(x, y, w, h, color);
}

void display_draw_bitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color) {
  display_spi_draw_bitmap(x, y, bitmap, w, h, color);
}

void display_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  display_spi_draw_line(x0, y0, x1, y1, color);
}

void display_set_cursor(uint16_t x, uint16_t y) {
  display_spi_set_cursor(x, y);
}

void display_set_textcolor(uint16_t color) {
  display_spi_set_textcolor(color);
}

void display_set_textsize(uint8_t size) {
  display_spi_set_textsize(size);
}

void display_prints(const char * text) {
  display_spi_prints(text);
}

void display_println(const char * text) {
  display_spi_println(text);
}

void display_print8(uint8_t value) {
  display_spi_print8(value);
}

void display_print16(uint16_t value) {
  display_spi_print16(value);
}

void display_print32(uint32_t value) {
  display_spi_print32(value);
}

uint16_t display_get_cursor_x() {
  return display_spi_get_cursor_x();
}

uint16_t display_get_cursor_y() {
  return display_spi_get_cursor_y();
}
#endif