#ifndef PAGE_FLASH_H_
#define PAGE_FLASH_H_

#include "stdbool.h"

bool flash_page_refresh();
bool flash_page_on_left();
bool flash_page_on_right();
bool flash_page_on_click();

#endif /* PAGE_FLASH_H_ */
