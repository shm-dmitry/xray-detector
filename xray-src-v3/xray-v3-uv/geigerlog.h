#pragma once

#define GEIGERLOG_ENABLED true

#if GEIGERLOG_ENABLED
void geigerlog_on_main_loop();
void isrcall__geigerlog_on_one_second();
#endif
