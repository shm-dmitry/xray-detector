#include "ui_manager.h"

#include "ui_frame_mainpage.h"
#include "ui_frame_borders.h"
#include "ui_frame_batstate.h"

#include "display.h"
#include "user_input.h"

#include "Arduino.h"

#define UI_MANAGER_REFRESH_DELAY (1000)

uint32_t ui_manager_next_refresh = 0;
bool ui_manager_active = false;
uint8_t current_page = 0;

bool ui_manager_init_radpage();

void ui_manager_init() {
  display_init();
}

void ui_manager_on_main_loop() {
  if (!ui_manager_active) {
    return;
  }

  bool pressed = user_input_pressed();
  if (pressed) {
    ui_manager_next_refresh = 0;

    current_page++;
    if (current_page > 2) {
      current_page = 0;
    }

    ui_borders_cleanup_working_area();
    
    if (!ui_manager_init_radpage()) {
      ui_frame_batstate_init();
    }    
  }

  uint32_t now = millis();
  if (now > ui_manager_next_refresh || ui_manager_next_refresh - now > UI_MANAGER_REFRESH_DELAY) {
    ui_borders_refresh();
    if (current_page < 2) {
      ui_frame_mainpage_refresh();
    } else {
      ui_frame_batstate_refresh();
    }

    ui_manager_next_refresh = now + UI_MANAGER_REFRESH_DELAY;
  }
}

void ui_manager_wakeup() {
  display_on();
  display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_BLACK);

  ui_borders_init();
  ui_manager_init_radpage();
  ui_frame_batstate_init();

  ui_manager_next_refresh = 0;
  ui_manager_active = true;
}

void ui_manager_go_sleep() {
  display_off();
  ui_manager_active = false;
}

bool ui_manager_init_radpage() {
  if (current_page == 0) {
    ui_frame_mainpage_init_gamma();
    return true;
  } else if (current_page == 1) {
    ui_frame_mainpage_init_betta();
    return true;
  }

  return false;
}

