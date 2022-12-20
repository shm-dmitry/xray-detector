#include "pages_control.h"

#include "page_rad.h"
#include "page_flash.h"
#include "userinput.h"
#include "stdint.h"
#include "page.h"
#include "display.h"

uint8_t pages_control_current_page = 0;

#define PAGES_COUNT 2
IPage * pages[2] = { new RadPage(), new FlashPage() };

void pages_control_init() {
  for (uint8_t i = 0; i<PAGES_COUNT; i++) {
    pages[i]->init();
  }

  pages_control_current_page = 0;
  pages[pages_control_current_page]->refresh();
}

void pages_change_screen(bool onleft) {
  if (onleft) {
    if (pages_control_current_page == 0) {
      pages_control_current_page = PAGES_COUNT - 1;
    } else {
      pages_control_current_page--;
    }
  } else {
    if (pages_control_current_page == PAGES_COUNT - 1) {
      pages_control_current_page = 0;
    } else {
      pages_control_current_page++;
    }
  }

  pages[pages_control_current_page]->refresh();
}

void pages_on_main_loop() {
  bool on = display_is_on();
  
  if (userinput_is_move_left()) {
    if (on) {
      if (!pages[pages_control_current_page]->on_left()) {
        pages_change_screen(true);
      }
    } else {
      display_on();
    }
  }
  if (userinput_is_move_right()) {
    if (on) {
      if (!pages[pages_control_current_page]->on_right()) {
        pages_change_screen(false);
      }
    } else {
      display_on();
    }
  }
  if (userinput_is_click()) {
    if (on) {
      pages[pages_control_current_page]->on_click();
    } else {
      display_on();
    }
  }

  if (on) {
    pages[pages_control_current_page]->refresh();
  }
}
