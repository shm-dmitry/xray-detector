#include "gui_manager.h"
#include "gui_ipagecallback.h"
#include "gui_page_borders.h"
#include "gui_page_rad.h"
#include "gui_page_flash.h"
#include "gui_page_bat.h"
#include "gui_page_settings.h"
#include "userinput.h"
#include "display.h"
#include "stddef.h"

#define GUI_PAGE_RAD   0

#define GUI_PAGE_ACTION_REFRESH 0
#define GUI_PAGE_ACTION_ONMOVE  1
#define GUI_PAGE_ACTION_ONCLICK 2
#define GUI_PAGE_ACTION_ONRESET 3

#define PAGES_COUNT    4
#define PAGE_CALLBACKS (GUI_PAGE_ACTION_ONRESET + 1)

uint8_t gui_page_current = 0;

// this "vtable" saves RAM and ROM against classes with virtual methods
const t_page_callback PAGES_VTABLE[PAGES_COUNT][PAGE_CALLBACKS] = {
  {&gui_rad_page_refresh,      NULL,                       &gui_rad_page_onclick,       &gui_rad_page_onwakeup},
  {&gui_flash_page_refresh,    &gui_flash_page_on_move,    &gui_flash_page_on_click,    &gui_flash_page_onwakeup},
  {&gui_bat_page_refresh,      NULL,                       NULL,                        &gui_bat_page_onwakeup},
  {&gui_settings_page_refresh, &gui_settings_page_on_move, &gui_settings_page_on_click, &gui_settings_page_onwakeup},
};

void gui_manager_init() {
  gui_page_current = 0xFF;
}

void gui_manager_on_main_loop() {
  if (!display_is_on()) {
    return;
  }

  uint8_t fullrefresh = 0x00;
  if (gui_page_current == 0xFF) {
    gui_page_current = GUI_PAGE_RAD;
    fullrefresh = USERINPUT_MOVE_RIGHT;
  }

  uint8_t move = userinput_get_move();
  if (move != USERINPUT_MOVE_NONE) {
    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONMOVE];
    if (func == NULL || !func(move)) {
      if (move == USERINPUT_MOVE_LEFT || move == USERINPUT_MOVE_LEFTFAST) {
        if (gui_page_current == 0) {
          gui_page_current = PAGES_COUNT - 1;
        } else {
          gui_page_current--;
        }

        fullrefresh = USERINPUT_MOVE_LEFT;
      } else {
        if (gui_page_current == PAGES_COUNT - 1) {
          gui_page_current = 0;
        } else {
          gui_page_current++;
        }

        fullrefresh = USERINPUT_MOVE_RIGHT;
      }
    }
  }

  if (userinput_is_click()) {
    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONCLICK];
    if (func != NULL) {
      func(0x00);
    }
  }

  if (fullrefresh) {
    if (display_is_on()) {
      display_fill_rect(0, 8, 160, 128-8*2, DISPLAY_BLACK);
    }
  }

  PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_REFRESH](fullrefresh);

  gui_borders_refresh(gui_page_current, PAGES_COUNT);
}

void gui_manager_on_wakeup() {
  for (uint8_t i = 0; i<PAGES_COUNT; i++) {
    t_page_callback func = PAGES_VTABLE[i][GUI_PAGE_ACTION_ONRESET];
    if (func != NULL) {
      func(0x00);
    }
  }

  gui_borders_onwakeup();
  PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_REFRESH](USERINPUT_MOVE_RIGHT);
}

void gui_manager_openrad() {
  if (gui_page_current != GUI_PAGE_RAD) {
    gui_page_current = GUI_PAGE_RAD;
    PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_REFRESH](0xFF);
    gui_borders_refresh(gui_page_current, PAGES_COUNT);
  }
}