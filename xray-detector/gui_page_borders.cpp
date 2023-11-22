#include "gui_page_borders.h"
#include "display.h"
#include "clock.h"
#include "charger_control.h"
#include "alarm_manager.h"
#include "gui_images.h"
#include "rad_control.h"

#define PAGE_BORDERS_BAT_PX       40
#define PAGE_BORDERS_BAT_HIGH     ((70 * PAGE_BORDERS_BAT_PX) / 100)
#define PAGE_BORDERS_BAT_MEDIUM   ((30 * PAGE_BORDERS_BAT_PX) / 100)

uint16_t page_borders_hash_cal  = 0xFFFF;
uint8_t  page_borders_tab       = 0xFF;
uint8_t  page_borders_bat_px    = 0xFF;
uint8_t  page_borders_dose_flag = 0xFF;

bool gui_borders_check_calchash(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
  uint16_t hash = year + month * 3 + day * 5 + hour * 7 + minute * 11;
  if (hash != page_borders_hash_cal) {
    page_borders_hash_cal = hash;
    return true;
  } else {
    return false;
  }
}

void gui_borders_show_date(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);
void gui_borders_show_tab(uint8_t currentpage, uint8_t pagescount);
void gui_borders_show_bat_pc(uint8_t voltagepc);
void gui_borders_show_dose_flag(uint8_t voltagepc);

void gui_borders_refresh(uint8_t currentpage, uint8_t pagescount) {
  if (!display_is_on()) {
    return;
  }

  uint16_t year = 0;
  uint8_t month = 0;
  uint8_t day = 0;
  uint8_t hour = 0; 
  uint8_t minute = 0;

  clock_get_time(year, month, day, hour, minute);

  uint8_t dose = alarm_manager_dose2level(rad_control_dose());

  uint8_t voltagepc = (uint8_t)(((uint16_t)charger_control_get_voltage_pc() * (uint16_t)PAGE_BORDERS_BAT_PX) / (uint16_t)100);

  if (gui_borders_check_calchash(year, month, day, hour, minute)) {
    gui_borders_show_date(year, month, day, hour, minute);
  }

  if (page_borders_tab != currentpage) {
    page_borders_tab = currentpage;
    gui_borders_show_tab(currentpage, pagescount);
  }

  if (page_borders_bat_px != voltagepc) {
    page_borders_bat_px = voltagepc;
    gui_borders_show_bat_pc(voltagepc);
  }

  if (page_borders_dose_flag != dose) {
    page_borders_dose_flag = dose;
    gui_borders_show_dose_flag(dose);
  }
}

void gui_borders_show_date(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
  display_set_cursor(0, 0);
  display_fill_rect(0, 0, 16*6*2, 7*2, DISPLAY_BLACK);
  display_set_textcolor(DISPLAY_WHITE);
  display_set_textsize(2);

  if (hour <= 9) {
    display_prints("0");
  } 
  display_print8(hour);

  if (minute <= 9) {
    display_prints(":0");
  } else {
    display_prints(":");
  }
  display_print8(minute);

  display_prints(" ");

  display_print16(year);
  if (month <= 9) {
    display_prints("-0");
    display_print8(month);
  } else {
    display_prints("-");
    display_print8(month);
  }

  if (day <= 9) {
    display_prints("-0");
    display_print8(day);
  } else {
    display_prints("-");
    display_print8(day);
  }
}

void gui_borders_show_tab(uint8_t currentpage, uint8_t pagescount) {
  for (uint8_t i = 0; i<pagescount; i++) {
    display_draw_bitmap(30 + i * 60, DISPLAY_HEIGHT-8*2, IMG_ICONS[i].img, IMG_ICONS[i].w, IMG_ICONS[i].h, i == currentpage ? DISPLAY_GREEN : DISPLAY_WHITE);
    if (i != pagescount - 1) {
      display_set_cursor(30 + i * 60 + 32, DISPLAY_HEIGHT-8*2);
      display_set_textcolor(DISPLAY_WHITE);
      display_set_textsize(2);
      display_prints("|");
    }
  }
}

void gui_borders_show_bat_pc(uint8_t voltagepc) {
  uint16_t color = voltagepc >= PAGE_BORDERS_BAT_HIGH ? DISPLAY_GREEN : 
                  (voltagepc >= PAGE_BORDERS_BAT_MEDIUM ? DISPLAY_WHITE : DISPLAY_RED);

  if (voltagepc == 38) {
    voltagepc = 40;
  }

  if (voltagepc > 0) {
    display_fill_rect(DISPLAY_WIDTH-40-4, 0, voltagepc, 16, color);
  }

  if (voltagepc < 38) {
    display_fill_rect(DISPLAY_WIDTH-40-4 + voltagepc, 0, 40 - voltagepc, 16, DISPLAY_BLACK);
    display_draw_rect(DISPLAY_WIDTH-40-4 + voltagepc, 0, 40 - voltagepc, 16, color);
  }
  
  display_fill_rect(DISPLAY_WIDTH-4, 4, 4, 8, color);
}

void gui_borders_show_dose_flag(uint8_t doseflag) {
  display_draw_bitmap(230, 0, IMG_XRAY_SMALL, IMG_XRAY_SMALL_W, IMG_XRAY_SMALL_H, doseflag == 0 ? DISPLAY_GREEN : (doseflag == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
}

void gui_borders_onwakeup() {
  page_borders_hash_cal  = 0xFFFF;
  page_borders_tab       = 0xFF;
  page_borders_bat_px    = 0xFF;
  page_borders_dose_flag = 0xFF;
}
