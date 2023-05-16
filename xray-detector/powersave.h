#ifndef POWERSAVE_H_
#define POWERSAVE_H_

#include "stdbool.h"

bool powersave_on_main_loop();
void powersave_wakeup();
bool powersave_is_on();

void isrcall_powersave_leave_standby();

#endif /* POWERSAVE_H_ */
