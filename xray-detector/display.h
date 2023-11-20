#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "stdint.h"

#define DISPLAY_WHITE  0xFFFF
#define DISPLAY_BLACK  0x0000
#define DISPLAY_GREEN  0x07E0
#define DISPLAY_RED    0xF800
#define DISPLAY_YELLOW 0xFFE0
#define DISPLAY_GRAY   0xC659

void display_init();
void display_on();
void display_off();
bool display_is_on();

void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_draw_bitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color);
void display_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void display_set_cursor(uint16_t x, uint16_t y);
void display_set_textcolor(uint16_t color);
void display_set_textsize(uint8_t size);
void display_prints(const char * text);
void display_println(const char * text);
void display_print8(uint8_t value);
void display_print16(uint16_t value);
void display_print32(uint32_t value);
uint16_t display_get_cursor_x();
uint16_t display_get_cursor_y();

#endif /* DISPLAY_H_ */
