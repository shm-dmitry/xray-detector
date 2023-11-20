/**
This code based on projects:
- https://github.com/adafruit/Adafruit_ILI9341
- https://github.com/adafruit/Adafruit-GFX-Library
*/

#ifndef DISPLAY_SPI_H_
#define DISPLAY_SPI_H_

#include "stdint.h"

void display_spi_init(uint8_t dc);
void display_spi_send_command(uint8_t command, const uint8_t * data = 0, uint8_t datasize = 0);
void display_spi_send_command_pgm(uint8_t command, const uint8_t * data = 0, uint8_t datasize = 0);

void display_spi_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_spi_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_spi_draw_bitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color);
void display_spi_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void display_spi_set_cursor(uint16_t x, uint16_t y);
void display_spi_set_textcolor(uint16_t color);
void display_spi_set_textsize(uint8_t size);
void display_spi_prints(const char * text);
void display_spi_println(const char * text);
void display_spi_print8(uint8_t value);
void display_spi_print16(uint16_t value);
void display_spi_print32(uint32_t value);
uint16_t display_spi_get_cursor_x();
uint16_t display_spi_get_cursor_y();

#endif /* DISPLAY_SPI_H_ */
