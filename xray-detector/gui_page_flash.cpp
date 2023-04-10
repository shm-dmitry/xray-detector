#include "gui_page_flash.h"
#include "Arduino.h"
#include "svf_control.h"
#include "userinput.h"
#include "display.h"
#include "gui_images.h"

uint8_t gui_flash_pc = 0x00;
uint8_t gui_flash_prev_pc = 0x00;
bool gui_flash_editing = false;
bool gui_flash_editing_prev = false;

bool gui_flash_page_refresh(uint8_t data) {
  if (!display_is_on()) {
    return false;
  }

  if (data) {
    display_draw_rect(20, 40, 50, 20, DISPLAY_WHITE);
    display_draw_line(20+50, 40-1, 20+50+10, 40-5-1, DISPLAY_WHITE);
    display_draw_line(20+50, 40+20, 20+50+10, 40+20+5, DISPLAY_WHITE);
    display_draw_line(20+50+10, 40-5, 20+50+10, 40+20+5, DISPLAY_WHITE);
    
    display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE, IMG_CIRCLE_H, IMG_CIRCLE_W, DISPLAY_GRAY);
    display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE_LOW, IMG_CIRCLE_LOW_H, IMG_CIRCLE_LOW_W, DISPLAY_BLACK);

    display_draw_bitmap(20+50+2, 40+6, IMG_TRIANGLE, IMG_TRIANGLE_H, IMG_TRIANGLE_W, DISPLAY_GRAY);
  }

  if (gui_flash_prev_pc != gui_flash_pc || gui_flash_editing_prev != gui_flash_editing || data) {
    if (gui_flash_pc == 0x00) {
      display_draw_bitmap(20+50+2, 40+6, IMG_TRIANGLE, IMG_TRIANGLE_H, IMG_TRIANGLE_W, DISPLAY_GRAY);
  
      display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE, IMG_CIRCLE_H, IMG_CIRCLE_W, DISPLAY_GRAY);
      display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE_LOW, IMG_CIRCLE_LOW_H, IMG_CIRCLE_LOW_W, DISPLAY_BLACK);

      display_fill_rect(29, 40+20+20, 102, 10, DISPLAY_BLACK);
      if (gui_flash_editing) {
        display_draw_rect(29, 40+20+20, 102, 10, DISPLAY_RED);
      }
    } else {
      display_draw_bitmap(20+50+2, 40+6, IMG_TRIANGLE, IMG_TRIANGLE_H, IMG_TRIANGLE_W, DISPLAY_YELLOW);

      display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE, IMG_CIRCLE_H, IMG_CIRCLE_W, DISPLAY_WHITE);
      display_draw_bitmap(20+50/2-4, 40+6, IMG_CIRCLE_LOW, IMG_CIRCLE_LOW_H, IMG_CIRCLE_LOW_W, DISPLAY_GREEN);

      if (gui_flash_editing) {
        if (!gui_flash_editing_prev) {
          display_draw_rect(29, 40+20+20, 102, 10, DISPLAY_WHITE);
          display_fill_rect(29+1, 40+20+20+1, gui_flash_prev_pc, 8, DISPLAY_YELLOW);
        }

        if (gui_flash_prev_pc < gui_flash_pc) {
          display_fill_rect(29+1+gui_flash_prev_pc, 40+20+20+1, gui_flash_pc - gui_flash_prev_pc, 8, DISPLAY_YELLOW);
        } else {
          display_fill_rect(29+1+gui_flash_pc, 40+20+20+1, gui_flash_prev_pc - gui_flash_pc, 8, DISPLAY_BLACK);
        }
      } else if (gui_flash_editing_prev) {
        display_fill_rect(29, 40+20+20, 102, 10, DISPLAY_BLACK);
      }
    }

    display_set_cursor(20+50+30, 40+3);
    display_set_textsize(2);
    display_set_textcolor(DISPLAY_BLACK);
    if (gui_flash_prev_pc == 0) {
      display_prints(" OFF");
    } else {
      display_print8(gui_flash_prev_pc);
      if (gui_flash_prev_pc < 100) {
        display_prints(" ");
      }
      display_prints("%");
    }
    display_set_cursor(20+50+30, 40+3);
    display_set_textcolor(DISPLAY_WHITE);
    if (gui_flash_pc == 0) {
      display_prints(" OFF");
    } else {
      display_print8(gui_flash_pc);
      if (gui_flash_pc < 100) {
        display_prints(" ");
      }
      display_prints("%");
    }

    gui_flash_prev_pc = gui_flash_pc;
    gui_flash_editing_prev = gui_flash_editing;
  }

  return true;
}

bool gui_flash_page_on_move(uint8_t data) {
  if (!gui_flash_editing) {
    return false;
  }

  int8_t delta = 0;
  if (data == USERINPUT_MOVE_LEFTFAST) {
    delta = -20;
  } else if (data == USERINPUT_MOVE_LEFT) {
    delta = -10;
  } else if (data == USERINPUT_MOVE_RIGHTFAST) {
    delta = +20;
  } else if (data == USERINPUT_MOVE_RIGHT) {
    delta = +10;
  }

  if (delta == 0) {
    return false;
  }

  if (delta < 0) {
    if (gui_flash_pc < -delta) {
      gui_flash_pc = 0;
    } else {
      gui_flash_pc += delta;
    }
  } else {
    if (gui_flash_pc + delta > 100) {
      gui_flash_pc = 100;
    } else {
      gui_flash_pc += delta;
    }
  }

  svf_control_flash(gui_flash_pc);

  return true;
}

bool gui_flash_page_on_click(uint8_t data) {
  gui_flash_editing = !gui_flash_editing;

  if (gui_flash_pc == 0x00 && gui_flash_editing) {
    gui_flash_pc = 50;
    svf_control_flash(gui_flash_pc);
  }
}

