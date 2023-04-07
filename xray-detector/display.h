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

Adafruit_SPITFT * display_get_object();

#endif /* DISPLAY_H_ */
