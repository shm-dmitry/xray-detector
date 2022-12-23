#include "pages_control.h"

#include "page_rad.h"
#include "page_flash.h"
#include "page_settings.h"
#include "page_bat.h"
#include "userinput.h"
#include "stdint.h"
#include "page.h"
#include "display.h"

uint8_t pages_current = 0;

#define PAGES_CONTROL_RAD 0

// this 'vtable' saves about 500 bytes ROM & 20 bytes RAM against variant with classes and virtual methods
#define PAGES_COUNT 4
const t_page_action pages_vtable[PAGES_COUNT][PAGE_ACTION_FLAG__MAXVAL] = {
  {NULL, NULL, &page_rad_onclick, &page_rad_refresh, NULL, &page_rad_need_refresh},
  {&flash_page_on_left, &flash_page_on_right, &flash_page_on_click, &flash_page_refresh, &flash_page_init, NULL},
  {NULL, NULL, NULL, &page_bat_refresh, NULL, NULL},
  {&settings_page_on_left, &settings_page_on_right, &settings_page_on_click, &settings_page_refresh, &settings_page_init, NULL},
};

void pages_control_init() {
  for (uint8_t i = 0; i<PAGES_COUNT; i++) {
    page_action_process(pages_vtable[i],           _BV(PAGE_ACTION_FLAG_INIT));
  }

  pages_current = 0;
  page_action_process(pages_vtable[pages_current], _BV(PAGE_ACTION_FLAG_REFRESH));
}

void pages_control_openrad() {
  if (pages_current != PAGES_CONTROL_RAD) {
    page_action_process(pages_vtable[pages_current], _BV(PAGE_ACTION_FLAG_INIT));
  }
  pages_current = 0;
}

void pages_change_screen(bool onleft) {
  if (onleft) {
    if (pages_current == 0) {
      pages_current = PAGES_COUNT - 1;
    } else {
      pages_current--;
    }
  } else {
    if (pages_current == PAGES_COUNT - 1) {
      pages_current = 0;
    } else {
      pages_current++;
    }
  }

  page_action_process(pages_vtable[pages_current], _BV(PAGE_ACTION_FLAG_REFRESH));
}

void pages_on_main_loop() {
  if (!display_is_on()) {
    return;
  }
  
  uint8_t flags = 
    (userinput_is_move_left()  ? _BV(PAGE_ACTION_FLAG_ISLEFT)  : 0) | 
    (userinput_is_move_right() ? _BV(PAGE_ACTION_FLAG_ISRIGHT) : 0) | 
    (userinput_is_click()      ? _BV(PAGE_ACTION_FLAG_ISCLICK) : 0);

  if (flags == 0) {
    if (page_action_process(pages_vtable[pages_current], _BV(PAGE_ACTION_FLAG_NEEDRF))) {
      page_action_process(pages_vtable[pages_current],   _BV(PAGE_ACTION_FLAG_REFRESH));
    }
  } else {
    if (page_action_process(pages_vtable[pages_current], flags)) {
      page_action_process(pages_vtable[pages_current],   _BV(PAGE_ACTION_FLAG_REFRESH));
    } else {
      if ((flags & _BV(PAGE_ACTION_FLAG_ISLEFT)) || 
          (flags & _BV(PAGE_ACTION_FLAG_ISRIGHT))) {
         pages_change_screen(flags & _BV(PAGE_ACTION_FLAG_ISLEFT));
      }
    }
  }
}
