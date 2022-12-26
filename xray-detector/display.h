#ifndef DISPLAY_H_
#define DISPLAY_H_

#define DISPLAY_SIMUL false

#if DISPLAY_SIMUL
#include <Adafruit_ILI9341.h>
#define DISPLAY_WHITE ILI9341_WHITE
#define DISPLAY_BLACK ILI9341_BLACK
#else
#include <Adafruit_ST7735.h>
#define DISPLAY_WHITE ST77XX_WHITE
#define DISPLAY_BLACK ST77XX_BLACK
#endif


void display_init();
void display_on();
void display_off();
bool display_is_on();

Adafruit_SPITFT * display_get_object();

#endif /* DISPLAY_H_ */
