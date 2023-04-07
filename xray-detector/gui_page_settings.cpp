#include "gui_page_settings.h"

#include "display.h"
#include "eeprom_control.h"
#include "uv_control.h"
#include "alarm_manager.h"
#include "gui_images.h"
#include "userinput.h"
#include "clock.h"

#define SETTINGS_PAGE_UV_FREQ 0
#define SETTINGS_PAGE_UV_DUTY 1
#define SETTINGS_PAGE_UV_ON   2
#define SETTINGS_PAGE_DATE    3
#define SETTINGS_PAGE_TIME    4
#define SETTINGS_PAGE_ALARM_H 5
#define SETTINGS_PAGE_ALRM_L1 6
#define SETTINGS_PAGE_ALRM_L2 7
#define SETTINGS_PAGE_ALRM_NI 8

#define SETTINGS_PAGE_MINVAL  SETTINGS_PAGE_UV_FREQ
#define SETTINGS_PAGE_MAXVAL  SETTINGS_PAGE_ALRM_NI

#define SETTINGS_PAGE_UNCHANGED 0xFF

#define SETTINGS_PAGE_EDIT_DATETIME_UNCHANGED 0
#define SETTINGS_PAGE_EDIT_DATETIME_YEAR      1
#define SETTINGS_PAGE_EDIT_DATETIME_MONTH     2
#define SETTINGS_PAGE_EDIT_DATETIME_DAY       3
#define SETTINGS_PAGE_EDIT_DATETIME_HOUR      4
#define SETTINGS_PAGE_EDIT_DATETIME_MINUTE    5

uint8_t menu_actual_prev   = SETTINGS_PAGE_MINVAL;
uint8_t menu_actual        = SETTINGS_PAGE_MINVAL;
uint8_t menu_change_value = SETTINGS_PAGE_UNCHANGED;
bool menu_refreshed        = false;
uint8_t menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_UNCHANGED;

void gui_settings_page_print(Adafruit_SPITFT * tft, bool force, uint8_t index, uint8_t value, const char * text);
void gui_settings_page_print_onoff(Adafruit_SPITFT * tft, bool force, uint8_t index, bool value);
void gui_settings_page_print_date(Adafruit_SPITFT * tft, bool force, uint8_t index);
void gui_settings_page_print_time(Adafruit_SPITFT * tft, bool force, uint8_t index);

bool gui_settings_page_refresh(uint8_t data) {
  Adafruit_SPITFT * tft = display_get_object();
  if (tft == NULL) {
    return false;
  }

  tft->setCursor(0, 18);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(1);

  if (data) {
    if (data == USERINPUT_MOVE_LEFT) {
      menu_actual = SETTINGS_PAGE_MAXVAL;
    } else {
      menu_actual = SETTINGS_PAGE_MINVAL;
    }

    tft->println("  UV freq: ");
    tft->println("  UV duty: ");
    tft->println("  UV: ");
    tft->println("  Date: ");
    tft->println("  Time: ");
    tft->println(" Alarms:");
    tft->println("  Level1: ");
    tft->println("  Level2: ");
    tft->println("  No Impuls: ");
  }

  if (menu_actual_prev != menu_actual || data) {
    tft->drawBitmap(3, 18 + 8 * menu_actual_prev, IMG_ARROW, IMG_ARROW_W, IMG_ARROW_H, DISPLAY_BLACK);
    tft->drawBitmap(3, 18 + 8 * menu_actual, IMG_ARROW, IMG_ARROW_W, IMG_ARROW_H, DISPLAY_YELLOW);
    menu_actual_prev = menu_actual;
  }

  if (data || !menu_refreshed) {
    gui_settings_page_print(tft, data, SETTINGS_PAGE_UV_FREQ, eeprom_control_get_freq(), "KHz");
    gui_settings_page_print(tft, data, SETTINGS_PAGE_UV_DUTY, eeprom_control_get_duty(), "%");
    gui_settings_page_print_onoff(tft, data, SETTINGS_PAGE_UV_ON, uv_control_is_on()); 
    gui_settings_page_print(tft, data, SETTINGS_PAGE_ALRM_L1, alarm_manager_getlevel(1), "uR"); 
    gui_settings_page_print(tft, data, SETTINGS_PAGE_ALRM_L2, alarm_manager_getlevel(2), "mR"); 
    gui_settings_page_print(tft, data, SETTINGS_PAGE_ALRM_NI, alarm_manager_getlevel(ALARM_MANAGER_NI_LEVEL), "sec"); 
    gui_settings_page_print_date(tft, data, SETTINGS_PAGE_DATE);
    gui_settings_page_print_time(tft, data, SETTINGS_PAGE_TIME);
  }

  menu_refreshed = true;

  return false;
}

void gui_settings_page_print(Adafruit_SPITFT * tft, bool force, uint8_t index, uint8_t value, const char * text) {
  tft->setCursor(80, 18 + 8 * index);
  tft->fillRect(tft->getCursorX(), tft->getCursorY(), 80, 8, DISPLAY_BLACK);

  if (menu_actual == index && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->setTextColor(DISPLAY_YELLOW);
    tft->print("<");
    tft->print(menu_change_value);
    tft->print(" ");
    tft->print(text);
    tft->print(">");
  } else {
    tft->setTextColor(DISPLAY_WHITE);
    tft->print(value);
    tft->print(" ");
    tft->print(text);
  }
}

void gui_settings_page_print_date(Adafruit_SPITFT * tft, bool force, uint8_t index) {
  tft->setCursor(80, 18 + 8 * index);
  tft->fillRect(tft->getCursorX(), tft->getCursorY(), 80, 8, DISPLAY_BLACK);
  
  if (menu_actual == index && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_YEAR) {
      tft->setTextColor(DISPLAY_YELLOW);
      tft->print("<");
      tft->print(menu_change_value);
      tft->print(">");
    } else {
      tft->setTextColor(DISPLAY_WHITE);
      tft->print(clock_get_component(CLOCK_COMPONENT_YEAR));
    }
    tft->setTextColor(DISPLAY_WHITE);
    tft->print("-");
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_MONTH) {
      tft->setTextColor(DISPLAY_YELLOW);
      tft->print("<");
      tft->print(menu_change_value);
      tft->print(">");
    } else {
      tft->setTextColor(DISPLAY_WHITE);
      uint8_t temp = clock_get_component(CLOCK_COMPONENT_MONTH);
      if (temp <= 9) {
        tft->print("0");
      }
      tft->print(temp);
    }
    tft->setTextColor(DISPLAY_WHITE);
    tft->print("-");
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_DAY) {
      tft->setTextColor(DISPLAY_YELLOW);
      tft->print("<");
      tft->print(menu_change_value);
      tft->print(">");
    } else {
      tft->setTextColor(DISPLAY_WHITE);
      uint8_t temp = clock_get_component(CLOCK_COMPONENT_DAY);
      if (temp <= 9) {
        tft->print("0");
      }
      tft->print(temp);
    }
  } else {
    tft->setTextColor(DISPLAY_WHITE);
    tft->print(clock_get_component(CLOCK_COMPONENT_YEAR));
    uint8_t temp = clock_get_component(CLOCK_COMPONENT_MONTH);
    if (temp <= 9) {
      tft->print("-0");
    } else {
      tft->print("-");
    }
    tft->print(temp);
    temp = clock_get_component(CLOCK_COMPONENT_DAY);
    if (temp <= 9) {
      tft->print("-0");
    } else {
      tft->print("-");
    }
    tft->print(temp);
  }
}

void gui_settings_page_print_time(Adafruit_SPITFT * tft, bool force, uint8_t index) {
  tft->setCursor(80, 18 + 8 * index);
  tft->fillRect(tft->getCursorX(), tft->getCursorY(), 80, 8, DISPLAY_BLACK);
  
  if (menu_actual == index && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_HOUR) {
      tft->setTextColor(DISPLAY_YELLOW);
      tft->print("<");
      tft->print(menu_change_value);
      tft->print(">");
    } else {
      tft->setTextColor(DISPLAY_WHITE);
      uint8_t temp = clock_get_component(CLOCK_COMPONENT_HOUR);
      if (temp <= 9) {
        tft->print("0");
      }
      tft->print(temp);
    }
    tft->setTextColor(DISPLAY_WHITE);
    tft->print(":");
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_MINUTE) {
      tft->setTextColor(DISPLAY_YELLOW);
      tft->print("<");
      tft->print(menu_change_value);
      tft->print(">");
    } else {
      tft->setTextColor(DISPLAY_WHITE);
      uint8_t temp = clock_get_component(CLOCK_COMPONENT_MINUTE);
      if (temp <= 9) {
        tft->print("0");
      }
      tft->print(temp);
    }
  } else {
    tft->setTextColor(DISPLAY_WHITE);
    uint8_t temp = clock_get_component(CLOCK_COMPONENT_HOUR);
    if (temp <= 9) {
      tft->print("0");
    }
    tft->print(temp);
    temp = clock_get_component(CLOCK_COMPONENT_MINUTE);
    if (temp <= 9) {
      tft->print(":0");
    } else {
      tft->print(":");
    }
    tft->print(temp);
  }
}

void gui_settings_page_print_onoff(Adafruit_SPITFT * tft, bool force, uint8_t index, bool value) {
  tft->setCursor(80, 18 + 8 * index);
  tft->fillRect(tft->getCursorX(), tft->getCursorY(), 80, 8, DISPLAY_BLACK);
  
  if (menu_actual == index && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->setTextColor(DISPLAY_YELLOW);
    tft->print("<");
    tft->print(value ? "ON" : "OFF");
    tft->print(">");
  } else {
    tft->setTextColor(DISPLAY_WHITE);
    tft->print(value ? "ON" : "OFF");
  }
}

bool gui_settings_page_on_left(bool fast) {
  if (menu_actual == SETTINGS_PAGE_UV_FREQ && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value -= fast ? 3 : 1;
    if (menu_change_value < 60) {
      menu_change_value = 60;
    }

    uv_control_change_pwm_with_testrun(menu_change_value, eeprom_control_get_duty());

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_UV_DUTY && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    uint8_t delta = fast ? 3 : 1;
    if (menu_change_value < delta) {
      menu_change_value = 1;
    } else {
      menu_change_value -= delta;
    }

    uv_control_change_pwm_with_testrun(eeprom_control_get_freq(), menu_change_value);

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_UV_ON && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (uv_control_is_on()) {
      uv_control_disable_pwm();
    } else {
      uv_control_enable_pwm();
    }

    menu_refreshed = false;
  } else if ((menu_actual == SETTINGS_PAGE_ALRM_L1 || menu_actual == SETTINGS_PAGE_ALRM_L2 || menu_actual == SETTINGS_PAGE_ALRM_NI) && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    uint8_t delta = fast ? 10 : 1;
    if (menu_change_value < delta) {
      menu_change_value = 1;
    } else {
      menu_change_value -= delta;
    }

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_DATE && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    uint8_t delta = 0;
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_DAY) {
      delta = fast ? 3 : 1;
    } else {
      delta = 1;
    }

    if (menu_change_value <= delta) {
      menu_change_value = 1;
    } else {
      menu_change_value -= delta;
    }

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_TIME && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    uint8_t delta = fast ? 5 : 1;

    if (menu_change_value <= delta) {
      menu_change_value = 0;
    } else {
      menu_change_value -= delta;
    }

    menu_refreshed = false;
  } else {
    if (menu_actual == SETTINGS_PAGE_MINVAL) {
      return false;
    } else {
      menu_actual--;
      if (menu_actual == SETTINGS_PAGE_ALARM_H) {
        menu_actual--;
      }
    }
  }
  
  return true;
}

bool gui_settings_page_on_right(bool fast) {
  if (menu_actual == SETTINGS_PAGE_UV_FREQ && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value += fast ? 3 : 1;
    if (menu_change_value > 130) {
      menu_change_value = 130;
    }

    uv_control_change_pwm_with_testrun(menu_change_value, eeprom_control_get_duty());

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_UV_DUTY && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value += fast ? 3 : 1;
    if (menu_change_value > 30) {
      menu_change_value = 30;
    }

    uv_control_change_pwm_with_testrun(eeprom_control_get_freq(), menu_change_value);

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_UV_ON && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (uv_control_is_on()) {
      uv_control_disable_pwm();
    } else {
      uv_control_enable_pwm();
    }

    menu_refreshed = false;
  } else if ((menu_actual == SETTINGS_PAGE_ALRM_L1 || menu_actual == SETTINGS_PAGE_ALRM_L2 || menu_actual == SETTINGS_PAGE_ALRM_NI) && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value += fast ? 10 : 1;
    if (menu_change_value > 240) {
      menu_change_value = 240;
    }

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_DATE && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    uint8_t delta = 0;
    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_DAY) {
      delta = fast ? 3 : 1;
    } else {
      delta = 1;
    }

    menu_change_value += delta;

    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_MONTH && menu_change_value > 12) {
      menu_change_value = 12;
    } else if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_DAY && menu_change_value > clock_days_in_month()) {
      menu_change_value = clock_days_in_month();
    }

    menu_refreshed = false;
  } else if (menu_actual == SETTINGS_PAGE_TIME && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value += fast ? 5 : 1;

    if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_HOUR && menu_change_value >= 23) {
      menu_change_value = 23;
    } else if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_MINUTE && menu_change_value >= 59) {
      menu_change_value = 59;
    } 

    menu_refreshed = false;
  } else {
    if (menu_actual == SETTINGS_PAGE_MAXVAL) {
      return false;
    } else {
      menu_actual++;
      if (menu_actual == SETTINGS_PAGE_ALARM_H) {
        menu_actual++;
      }
    }
  }
  
  return true;
}

bool gui_settings_page_on_move(uint8_t data) {
  if (data == USERINPUT_MOVE_LEFT || data == USERINPUT_MOVE_LEFTFAST) {
    return gui_settings_page_on_left(data == USERINPUT_MOVE_LEFTFAST);
  } else {
    return gui_settings_page_on_right(data == USERINPUT_MOVE_RIGHTFAST);
  }
}

bool gui_settings_page_on_click(uint8_t data) {
  if (menu_actual == SETTINGS_PAGE_UV_FREQ) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_change_value = eeprom_control_get_freq();
    } else {
      eeprom_control_save_freq(menu_change_value);
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  } else if (menu_actual == SETTINGS_PAGE_UV_DUTY) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_change_value = eeprom_control_get_duty();
    } else {
      eeprom_control_save_duty(menu_change_value);
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  } else if (menu_actual == SETTINGS_PAGE_UV_ON) {
    menu_change_value = menu_change_value == SETTINGS_PAGE_UNCHANGED ? 1 : SETTINGS_PAGE_UNCHANGED;
  } else if (menu_actual == SETTINGS_PAGE_ALRM_L1 || menu_actual == SETTINGS_PAGE_ALRM_L2) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_change_value = alarm_manager_getlevel(menu_actual == SETTINGS_PAGE_ALRM_L1 ? 1 : 2);
    } else {
      if (menu_actual == SETTINGS_PAGE_ALRM_L1) {
        eeprom_control_save_alarm_levels(menu_change_value, alarm_manager_getlevel(2));
      } else {
        eeprom_control_save_alarm_levels(alarm_manager_getlevel(1), menu_change_value);
      }
      alarm_manager_refresh_levels();
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  } else if (menu_actual == SETTINGS_PAGE_ALRM_NI) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_change_value = alarm_manager_getlevel(ALARM_MANAGER_NI_LEVEL);
    } else {
      eeprom_control_set_noimpulse_seconds(menu_change_value);
      alarm_manager_refresh_levels();
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  } else if (menu_actual == SETTINGS_PAGE_DATE) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_YEAR;
      menu_change_value = clock_get_component(CLOCK_COMPONENT_YEAR);
    } else if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_YEAR) {
      clock_set_component(CLOCK_COMPONENT_YEAR, menu_change_value);
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_MONTH;
      menu_change_value = clock_get_component(CLOCK_COMPONENT_MONTH);
    } else if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_MONTH) {
      clock_set_component(CLOCK_COMPONENT_MONTH, menu_change_value);
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_DAY;
      menu_change_value = clock_get_component(CLOCK_COMPONENT_DAY);
    } else {
      clock_set_component(CLOCK_COMPONENT_DAY, menu_change_value);
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_UNCHANGED;
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  } else if (menu_actual == SETTINGS_PAGE_TIME) {
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_HOUR;
      menu_change_value = clock_get_component(CLOCK_COMPONENT_HOUR);
    } else if (menu_edit_datetime == SETTINGS_PAGE_EDIT_DATETIME_HOUR) {
      clock_set_component(CLOCK_COMPONENT_HOUR, menu_change_value);
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_MINUTE;
      menu_change_value = clock_get_component(CLOCK_COMPONENT_MINUTE);
    } else {
      clock_set_component(CLOCK_COMPONENT_MINUTE, menu_change_value);
      menu_edit_datetime = SETTINGS_PAGE_EDIT_DATETIME_UNCHANGED;
      menu_change_value = SETTINGS_PAGE_UNCHANGED;
    }
  }

  menu_refreshed = false;
  
  return true;
}
