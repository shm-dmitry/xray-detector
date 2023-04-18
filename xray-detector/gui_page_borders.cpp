#include "gui_page_borders.h"
#include "display.h"
#include "clock.h"
#include "charger_control.h"
#include "alarm_manager.h"
#include "gui_images.h"
#include "rad_control.h"

#define PAGE_BORDERS_BAT_PX       20
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

  uint8_t voltagepc = charger_control_get_voltage_pc() * PAGE_BORDERS_BAT_PX / 100;

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
  display_fill_rect(0, 0, 16*6, 7, DISPLAY_BLACK);
  display_set_textcolor(DISPLAY_WHITE);
  display_set_textsize(1);

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
    display_draw_bitmap(20 + i * 30, 128-7, IMG_ICONS[i].img, IMG_ICONS[i].w, IMG_ICONS[i].h, i == currentpage ? DISPLAY_GREEN : DISPLAY_WHITE);
    if (i != pagescount - 1) {
      display_set_cursor(20+i * 30 + 16, 128-7);
      display_set_textcolor(DISPLAY_WHITE);
      display_set_textsize(1);
      display_prints("|");
    }
  }
}

void gui_borders_show_bat_pc(uint8_t voltagepc) {
  uint16_t color = voltagepc >= PAGE_BORDERS_BAT_HIGH ? DISPLAY_GREEN : 
                  (voltagepc >= PAGE_BORDERS_BAT_MEDIUM ? DISPLAY_WHITE : DISPLAY_RED);

  if (voltagepc == 19) {
    voltagepc = 20;
  }

  if (voltagepc > 0) {
    display_fill_rect(160-20-2, 0, voltagepc, 8, color);
  }

  if (voltagepc < 19) {
    display_fill_rect(160-20-2 + voltagepc, 0, 20 - voltagepc, 8, DISPLAY_BLACK);
    display_draw_rect(160-20-2 + voltagepc, 0, 20 - voltagepc, 8, color);
  }
  
  display_fill_rect(160-2, 2, 2, 4, color);
}

void gui_borders_show_dose_flag(uint8_t doseflag) {
  display_draw_bitmap(115, 0, IMG_XRAY_SMALL, IMG_XRAY_SMALL_W, IMG_XRAY_SMALL_H, doseflag == 0 ? DISPLAY_GREEN : (doseflag == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
}

void gui_borders_onwakeup() {
  page_borders_hash_cal  = 0xFFFF;
  page_borders_tab       = 0xFF;
  page_borders_bat_px    = 0xFF;
  page_borders_dose_flag = 0xFF;
}
