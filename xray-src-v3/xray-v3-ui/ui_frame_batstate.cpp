#include "ui_frame_batstate.h"

#include "bat_state.h"
#include "display.h"

uint16_t ui_frame_batstate_voltage = 0;
uint16_t ui_frame_batstate_current = 0;
uint8_t ui_frame_batstate_percent = 0;

void ui_frame_batstate_write_voltage(uint16_t value, bool hide);
void ui_frame_batstate_write_current(uint16_t value, bool hide);
void ui_frame_batstate_write_percent(uint8_t value, bool hide);

void ui_frame_batstate_refresh() {
  uint16_t v = bat_state_get_voltage();
  uint16_t a = bat_state_get_current();
  uint8_t pc = bat_state_get_percent();

  if (v != ui_frame_batstate_voltage) {
    ui_frame_batstate_write_voltage(ui_frame_batstate_voltage, true);
    ui_frame_batstate_write_voltage(v, false);
    ui_frame_batstate_voltage = v;
  }

  if (a != ui_frame_batstate_current) {
    ui_frame_batstate_write_current(ui_frame_batstate_current, true);
    ui_frame_batstate_write_current(a, false);
    ui_frame_batstate_current = a;
  }

  if (pc != ui_frame_batstate_percent) {
    ui_frame_batstate_write_percent(ui_frame_batstate_percent, true);
    ui_frame_batstate_write_percent(pc, false);
    ui_frame_batstate_percent = pc;
  }
}

void ui_frame_batstate_init() {
  ui_frame_batstate_voltage = 0;
  ui_frame_batstate_current = 0;
  ui_frame_batstate_percent = 0;
}

void ui_frame_batstate_write_voltage(uint16_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_set_textsize(2);
  display_set_cursor(10, 40);
  display_prints("V  = ");
  if (value == 0xFFFF) {
    display_prints("No data");
  } else {
    display_print16(value);
    display_prints("mV");
  }
}

void ui_frame_batstate_write_current(uint16_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_set_textsize(2);
  display_set_cursor(10, 80);
  display_prints("A  = ");
  if (value == 0xFFFF) {
    display_prints("No data");
  } else {
    display_print16(value);
    display_prints("mA");
  }
}

void ui_frame_batstate_write_percent(uint8_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_set_textsize(2);
  display_set_cursor(10, 120);
  display_prints("PC = ");
  if (value == 0xFF) {
    display_prints("No data");
  } else {
    display_print8(value);
    display_prints("%");
  }
}

