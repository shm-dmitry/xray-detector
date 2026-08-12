#pragma once

#define GEIGERLOG_ENABLED false

#if GEIGERLOG_ENABLED
void geigerlog_on_main_loop();
void geigerlog_enable(bool enable);
void isrcall__geigerlog_on_one_second();
#endif
