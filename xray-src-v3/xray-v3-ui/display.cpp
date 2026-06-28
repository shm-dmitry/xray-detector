#include "display.h"

#include "Arduino.h"

#include "display_st7789.h"
//#include "display_st7735.h"
//#include "display_ili9341.h"
#include "display_spi.h"

#define DISPLAY_DC_PIN      PIN_PC3
#define DISPLAY_ENABLE_PIN  PIN_PC1

void display_init() {
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  delay(1000);

  display_on();
}

void display_on() {
  if (display_is_on()) {
    return;
  }

  digitalWrite(DISPLAY_ENABLE_PIN, LOW);
  delay(30); // await for a power up

  display_st7789_init(DISPLAY_DC_PIN);
  //display_st7735_init(DISPLAY_DC_PIN);
  //display_ili9341_init(DISPLAY_DC_PIN);

  display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_BLACK);
}

void display_off() {
  digitalWrite(DISPLAY_ENABLE_PIN, HIGH);
  digitalWrite(DISPLAY_DC_PIN, LOW);
  delay(100);
}

bool display_is_on() {
  return digitalRead(DISPLAY_ENABLE_PIN) == LOW;
}

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
