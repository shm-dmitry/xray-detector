#ifndef DISPLAY_H_
#define DISPLAY_H_

#define DISPLAY_SIMUL true

#if DISPLAY_SIMUL
#include <Adafruit_ILI9341.h>
#define DISPLAY_WHITE  ILI9341_WHITE
#define DISPLAY_BLACK  ILI9341_BLACK
#define DISPLAY_GREEN  ILI9341_GREEN
#define DISPLAY_RED    ILI9341_RED
#define DISPLAY_YELLOW ILI9341_YELLOW
#define DISPLAY_GRAY   ILI9341_LIGHTGREY
#else
#include <Adafruit_ST7735.h>
#define DISPLAY_WHITE  ST77XX_WHITE
#define DISPLAY_BLACK  ST77XX_BLACK
#define DISPLAY_GREEN  ST77XX_GREEN
#define DISPLAY_RED    ST77XX_RED
#define DISPLAY_YELLOW ST77XX_YELLOW
#define DISPLAY_GRAY   0xC659
#endif

void display_init();
void display_on();
void display_off();
bool display_is_on();

void display_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void display_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color);
void display_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void display_set_cursor(uint8_t x, uint8_t y);
void display_set_textcolor(uint16_t color);
void display_set_textsize(uint8_t size);
void display_prints(const char * text);
void display_println(const char * text);
void display_print8(uint8_t value);
void display_print16(uint16_t value);
void display_print32(uint32_t value);
uint8_t display_get_cursor_x();
uint8_t display_get_cursor_y();

#endif /* DISPLAY_H_ */
