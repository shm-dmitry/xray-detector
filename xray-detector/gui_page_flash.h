#ifndef GUI_PAGE_FLASH_H_
#define GUI_PAGE_FLASH_H_

#include "stdbool.h"
#include "stdint.h"

bool gui_flash_page_refresh(uint8_t data);
bool gui_flash_page_on_move(uint8_t data);
bool gui_flash_page_on_click(uint8_t data);

#endif /* GUI_PAGE_FLASH_H_ */

