#include "ui_frame_mainpage.h"
#include "ui_frame_borders.h"
#include "ui_manager.h"

#include "display.h"
#include "user_input.h"

#include "Arduino.h"

#define UI_MANAGER_REFRESH_DELAY (1000)

uint32_t ui_manager_next_refresh = 0;
bool ui_manager_active = false;

void ui_manager_init() {
  display_init();
}

void ui_manager_on_main_loop() {
  if (!ui_manager_active) {
    return;
  }

  uint32_t now = millis();
  if (now > ui_manager_next_refresh || ui_manager_next_refresh - now > UI_MANAGER_REFRESH_DELAY) {
    ui_borders_refresh();
    ui_frame_mainpage_refresh();

    ui_manager_next_refresh = now + UI_MANAGER_REFRESH_DELAY;
  }
}

void ui_manager_wakeup() {
  display_on();
  display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_BLACK);

  ui_borders_init();
  ui_frame_mainpage_init();

  ui_manager_next_refresh = 0;
  ui_manager_active = true;
}

void ui_manager_go_sleep() {
  display_off();
  ui_manager_active = false;
}
