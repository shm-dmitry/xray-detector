#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Adafruit_ILI9341.h>

//#define DISPLAY_WHITE ST77XX_WHITE
//#define DISPLAY_BLACK ST77XX_BLACK
#define DISPLAY_WHITE ILI9341_WHITE
#define DISPLAY_BLACK ILI9341_BLACK

void display_init();
void display_on();
void display_off();
bool display_is_on();

Adafruit_SPITFT * display_get_object();

#endif /* DISPLAY_H_ */
