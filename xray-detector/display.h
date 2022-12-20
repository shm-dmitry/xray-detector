#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Adafruit_ST7735.h>

void display_init();
void display_on();
void display_off();
bool display_is_on();

Adafruit_ST7735 * display_get_object();

#endif /* DISPLAY_H_ */
