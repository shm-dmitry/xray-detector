#include "gui_page_rad_betta.h"
#include "rad_control.h"
#include "clock.h"
#include "display.h"

#define GUI_RAD_PAGE_BETTA_STAGE_INITR    0
#define GUI_RAD_PAGE_BETTA_STAGE_INIT     1
#define GUI_RAD_PAGE_BETTA_STAGE_INITP    2
#define GUI_RAD_PAGE_BETTA_STAGE_MEASURE1 3
#define GUI_RAD_PAGE_BETTA_STAGE_AWAIT2   4
#define GUI_RAD_PAGE_BETTA_STAGE_AWAIT2P  5
#define GUI_RAD_PAGE_BETTA_STAGE_MEASURE2 6
#define GUI_RAD_PAGE_BETTA_STAGE_DONE     7
#define GUI_RAD_PAGE_BETTA_STAGE_DONEP    8

#define GUI_RAD_PAGE_BETTA_MIN_IMPL       100
#define GUI_RAD_PAGE_BETTA_MAX_IMPL       0xFF00
#define GUI_RAD_PAGE_BETTA_MIN_TIME       30
#define GUI_RAD_PAGE_BETTA_MAX_TIME       200

#define GUI_RAD_PAGE_BETTA_CALC_IPH(counter, secs) (((uint32_t)counter * (uint32_t)60 * (uint32_t)60) / (uint32_t)secs)

uint16_t gui_rad_page_betta_usercounter_stage1 = RAD_CONTROL_USER_COUNTER_DISABLED;
uint16_t gui_rad_page_betta_usercounter_stage2 = RAD_CONTROL_USER_COUNTER_DISABLED;
uint32_t gui_rad_page_betta_start_measurement  = 0;
uint8_t  gui_rad_page_betta_prev_time          = 0xFF;
uint8_t  gui_rad_page_betta_stage1_time        = 0;

uint8_t gui_rad_page_betta_stage               = GUI_RAD_PAGE_BETTA_STAGE_INIT;

bool gui_rad_page_betta_check_measure_done();

bool gui_rad_page_betta_refresh(uint8_t data) {
  if (data) {
    gui_rad_page_betta_onwakeup(0);
  }

  if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_INITR) {
    display_fill_rect(0, 8, 160, 128-8*2, DISPLAY_BLACK);
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_INIT;
    gui_rad_page_betta_onwakeup(0);
  } 
  
  if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_INIT) {
    display_set_cursor(10, 20);
    display_set_textsize(1);
    display_set_textcolor(DISPLAY_WHITE);
    display_prints("Close protection cup");
    display_set_cursor(10, 20+16);
    display_prints("and click to start");
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_INITP;
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE1) {
    uint16_t value = rad_control_user_counter_getvalue();
    if (value != gui_rad_page_betta_usercounter_stage1) {
      if (gui_rad_page_betta_usercounter_stage1 == RAD_CONTROL_USER_COUNTER_DISABLED) {
        display_set_cursor(10, 20);
        display_set_textsize(1);
        display_set_textcolor(DISPLAY_BLACK);
        display_prints("Close protection cup");
        display_set_cursor(10, 20+16);
        display_prints("and click to start");

        display_set_textcolor(DISPLAY_WHITE);

        display_set_cursor(10, 20);
        display_set_textcolor(DISPLAY_YELLOW);
        display_prints("Gamma only");

        display_set_textcolor(DISPLAY_WHITE);
        display_set_cursor(20, 20+8);
        display_prints("Impulses:");
        display_set_cursor(20, 20+8+8);
        display_prints("Time:");
      } 

      display_set_cursor(20+10*6, 20+8);
      display_set_textcolor(DISPLAY_BLACK);
      display_print16(gui_rad_page_betta_usercounter_stage1);

      display_set_cursor(20+10*6, 20+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_print16(value);

      gui_rad_page_betta_usercounter_stage1 = value;
    }

    uint8_t passed = (clock_millis() - gui_rad_page_betta_start_measurement) / 1000;

    if (passed != gui_rad_page_betta_prev_time) {
      display_set_cursor(20+10*6, 20+8+8);
      display_set_textcolor(DISPLAY_BLACK);
      display_print16(gui_rad_page_betta_prev_time);
      display_prints(" sec");

      display_set_cursor(20+10*6, 20+8+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_print16(passed);
      display_prints(" sec");

      gui_rad_page_betta_prev_time = passed;
    }

    if (gui_rad_page_betta_check_measure_done()) {
      rad_control_user_counter_startstop(false);
      gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_AWAIT2;
      gui_rad_page_betta_stage1_time = gui_rad_page_betta_prev_time;

      display_set_cursor(20, 20+8+8+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_prints("Result: ");
      display_print32(GUI_RAD_PAGE_BETTA_CALC_IPH(gui_rad_page_betta_usercounter_stage1, gui_rad_page_betta_stage1_time));
      display_prints(" impl/hour");
    } 
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_AWAIT2) {
    display_set_cursor(10, 20+8+8+8+8+16);
    display_set_textsize(1);
    display_set_textcolor(DISPLAY_WHITE);
    display_prints("Open protection cup");
    display_set_cursor(10, 20+8+8+8+8+16+16);
    display_prints("and click to start");
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_AWAIT2P;
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE2) {
    uint16_t value = rad_control_user_counter_getvalue();
    if (value != gui_rad_page_betta_usercounter_stage2) {
      if (gui_rad_page_betta_usercounter_stage2 == RAD_CONTROL_USER_COUNTER_DISABLED) {
        display_set_cursor(10, 20+8+8+8+8+16);
        display_set_textsize(1);
        display_set_textcolor(DISPLAY_BLACK);
        display_prints("Open protection cup");
        display_set_cursor(10, 20+8+8+8+8+16+16);
        display_prints("and click to start");

        display_set_textcolor(DISPLAY_YELLOW);
        display_set_cursor(10, 20+8+8+8+8);
        display_prints("Gamma plus Betta");

        display_set_textcolor(DISPLAY_WHITE);
        display_set_cursor(20, 20+8+8+8+8+8);
        display_prints("Impulses:");
        display_set_cursor(20, 20+8+8+8+8+8+8);
        display_prints("Time:");
      } 
      
      display_set_cursor(20+10*6, 20+8+8+8+8+8);
      display_set_textcolor(DISPLAY_BLACK);
      display_print16(gui_rad_page_betta_usercounter_stage2);

      display_set_cursor(20+10*6, 20+8+8+8+8+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_print16(value);

      gui_rad_page_betta_usercounter_stage2 = value;
    }

    uint8_t passed = (clock_millis() - gui_rad_page_betta_start_measurement) / 1000;

    if (passed != gui_rad_page_betta_prev_time) {
      display_set_cursor(20+10*6, 20+8+8+8+8+8+8);
      display_set_textcolor(DISPLAY_BLACK);
      display_print16(gui_rad_page_betta_prev_time);
      display_prints(" sec");

      display_set_cursor(20+10*6, 20+8+8+8+8+8+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_print16(passed);
      display_prints(" sec");

      gui_rad_page_betta_prev_time = passed;
    }

    if (gui_rad_page_betta_check_measure_done()) {
      rad_control_user_counter_startstop(false);
      gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_DONE;

      display_set_cursor(20, 20+8+8+8+8+8+8+8);
      display_set_textcolor(DISPLAY_WHITE);
      display_prints("Result: ");
      display_print32(GUI_RAD_PAGE_BETTA_CALC_IPH(gui_rad_page_betta_usercounter_stage2, gui_rad_page_betta_prev_time));
      display_prints(" impl/hour");
    }
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_DONE) {
    display_set_cursor(5, 20+8+8+8+8+8+8+8+20);
    display_set_textsize(1);
    display_set_textcolor(DISPLAY_GREEN);
    uint32_t stage1 = GUI_RAD_PAGE_BETTA_CALC_IPH(gui_rad_page_betta_usercounter_stage1, gui_rad_page_betta_stage1_time);
    uint32_t stage2 = GUI_RAD_PAGE_BETTA_CALC_IPH(gui_rad_page_betta_usercounter_stage2, gui_rad_page_betta_prev_time);
    if (stage2 < stage1) {
      display_set_textcolor(DISPLAY_RED);
      display_prints("No betta detected.");
    } else {
      display_set_textcolor(DISPLAY_GREEN);
      display_prints("Betta: ");
      display_print16(stage2 - stage1);
      display_prints(" impl/hour");
    }

    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_DONEP;
  }

  return true;
}

bool gui_rad_page_betta_check_measure_done() {
  if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE1) {
    if (gui_rad_page_betta_usercounter_stage1 > GUI_RAD_PAGE_BETTA_MIN_IMPL && gui_rad_page_betta_prev_time > GUI_RAD_PAGE_BETTA_MIN_TIME) {
      return true;
    }

    if (gui_rad_page_betta_usercounter_stage1 > GUI_RAD_PAGE_BETTA_MAX_IMPL || gui_rad_page_betta_prev_time > GUI_RAD_PAGE_BETTA_MAX_TIME) {
      return true;
    }
  }

  if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE2) {
    if (gui_rad_page_betta_usercounter_stage2 > GUI_RAD_PAGE_BETTA_MIN_IMPL && gui_rad_page_betta_prev_time > GUI_RAD_PAGE_BETTA_MIN_TIME) {
      return true;
    }

    if (gui_rad_page_betta_usercounter_stage2 > GUI_RAD_PAGE_BETTA_MAX_IMPL || gui_rad_page_betta_prev_time > GUI_RAD_PAGE_BETTA_MAX_TIME) {
      return true;
    }
  }

  return false;
}

bool gui_rad_page_betta_onclick(uint8_t data) {
  if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_INITP) {
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_MEASURE1;
    rad_control_user_counter_startstop(true);
    gui_rad_page_betta_start_measurement = clock_millis();
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_AWAIT2P) {
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_MEASURE2;
    rad_control_user_counter_startstop(true);
    gui_rad_page_betta_start_measurement = clock_millis();
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_DONEP) {
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_INITR;
  } else if (gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE1 || gui_rad_page_betta_stage == GUI_RAD_PAGE_BETTA_STAGE_MEASURE2) {
    gui_rad_page_betta_stage = GUI_RAD_PAGE_BETTA_STAGE_INITR;
  }

  return true;
}

bool gui_rad_page_betta_onwakeup(uint8_t data) {
  rad_control_user_counter_startstop(false);
  gui_rad_page_betta_usercounter_stage1 = RAD_CONTROL_USER_COUNTER_DISABLED;
  gui_rad_page_betta_usercounter_stage2 = RAD_CONTROL_USER_COUNTER_DISABLED;
  gui_rad_page_betta_start_measurement  = 0;
  gui_rad_page_betta_prev_time          = 0xFF;
  gui_rad_page_betta_stage1_time        = 0;
  gui_rad_page_betta_stage              = GUI_RAD_PAGE_BETTA_STAGE_INIT;

  return true;
}
