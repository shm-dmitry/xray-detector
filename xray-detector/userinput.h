#ifndef USERINPUT_H_
#define USERINPUT_H_

#include "stdint.h"
#include "stdbool.h"

#define USERINPUT_MOVE_NONE      0
#define USERINPUT_MOVE_LEFT      1
#define USERINPUT_MOVE_LEFTFAST  2
#define USERINPUT_MOVE_RIGHT     3
#define USERINPUT_MOVE_RIGHTFAST 4

void userinput_init();

uint8_t userinput_get_move();
bool userinput_is_click();
bool userinput_is_wakeup();

void userinput_reset();

#endif /* USERINPUT_H_ */
