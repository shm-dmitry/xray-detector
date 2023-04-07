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

void gui_borders_show_date(Adafruit_SPITFT * tft, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);
void gui_borders_show_tab(Adafruit_SPITFT * tft, uint8_t currentpage, uint8_t pagescount);
void gui_borders_show_bat_pc(Adafruit_SPITFT * tft, uint8_t voltagepc);
void gui_borders_show_dose_flag(Adafruit_SPITFT * tft, uint8_t voltagepc);

void gui_borders_refresh(uint8_t currentpage, uint8_t pagescount) {
  Adafruit_SPITFT * tft = display_get_object();
  if (tft == NULL) {
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
    gui_borders_show_date(tft, year, month, day, hour, minute);
  }

  if (page_borders_tab != currentpage) {
    page_borders_tab = currentpage;
    gui_borders_show_tab(tft, currentpage, pagescount);
  }

  if (page_borders_bat_px != voltagepc) {
    page_borders_bat_px = voltagepc;
    gui_borders_show_bat_pc(tft, voltagepc);
  }

  if (page_borders_dose_flag != dose) {
    page_borders_dose_flag = dose;
    gui_borders_show_dose_flag(tft, dose);
  }
}

void gui_borders_show_date(Adafruit_SPITFT * tft, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
  tft->setCursor(0, 0);
  tft->fillRect(0, 0, 16*6, 7, DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(1);

  if (hour <= 9) {
    tft->print(0);
  } 
  tft->print(hour);

  if (minute <= 9) {
    tft->print(":0");
  } else {
    tft->print(":");
  }
  tft->print(minute);

  tft->print(" ");

  tft->print(year);
  if (month <= 9) {
    tft->print("-0");
    tft->print(month);
  } else {
    tft->print("-");
    tft->print(month);
  }

  if (day <= 9) {
    tft->print("-0");
    tft->print(day);
  } else {
    tft->print("-");
    tft->print(day);
  }
}

void gui_borders_show_tab(Adafruit_SPITFT * tft, uint8_t currentpage, uint8_t pagescount) {
  for (uint8_t i = 0; i<pagescount; i++) {
    tft->drawBitmap(20 + i * 30, 128-7, IMG_ICONS[i].img, IMG_ICONS[i].w, IMG_ICONS[i].h, i == currentpage ? DISPLAY_GREEN : DISPLAY_WHITE);
    if (i != pagescount - 1) {
      tft->setCursor(20+i * 30 + 16, 128-7);
      tft->setTextColor(DISPLAY_WHITE);
      tft->setTextSize(1);
      tft->print("|");
    }
  }
}

void gui_borders_show_bat_pc(Adafruit_SPITFT * tft, uint8_t voltagepc) {
  uint16_t color = voltagepc >= PAGE_BORDERS_BAT_HIGH ? DISPLAY_GREEN : 
                  (voltagepc >= PAGE_BORDERS_BAT_MEDIUM ? DISPLAY_WHITE : DISPLAY_RED);

  if (voltagepc == 19) {
    voltagepc = 20;
  }

  tft->fillRect(160-20-2, 0, voltagepc, 8, color);
  if (voltagepc < 19) {
    tft->fillRect(160-20-2 + voltagepc, 0, 20 - voltagepc, 8, DISPLAY_BLACK);
    tft->drawRect(160-20-2 + voltagepc, 0, 20 - voltagepc, 8, color);
  }
  tft->fillRect(160-2, 2, 2, 4, color);
}

void gui_borders_show_dose_flag(Adafruit_SPITFT * tft, uint8_t doseflag) {
  tft->drawBitmap(115, 0, IMG_XRAY_SMALL, IMG_XRAY_SMALL_W, IMG_XRAY_SMALL_H, doseflag == 0 ? DISPLAY_GREEN : (doseflag == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
}
