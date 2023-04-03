#include "page_settings.h"

#include "display.h"
#include "eeprom_control.h"
#include "uv_control.h"
#include "alarm_manager.h"

#define SETTINGS_PAGE_BACK    0
#define SETTINGS_PAGE_UV_FREQ 1
#define SETTINGS_PAGE_UV_DUTY 2
#define SETTINGS_PAGE_UV_ON   3
#define SETTINGS_PAGE_ALRM_L1 10
#define SETTINGS_PAGE_ALRM_L2 11
#define SETTINGS_PAGE_ALRM_NI 12

#define SETTINGS_PAGE_MINVAL  SETTINGS_PAGE_BACK
#define SETTINGS_PAGE_MAXVAL  SETTINGS_PAGE_ALRM_NI

#define SETTINGS_PAGE_UNCHANGED 0xFFFFFFFF

uint8_t menu_actual = SETTINGS_PAGE_BACK;
uint32_t menu_change_value = SETTINGS_PAGE_UNCHANGED;
bool menu_editing = false;

bool settings_page_init() {
  menu_actual = SETTINGS_PAGE_BACK;
  menu_change_value = SETTINGS_PAGE_UNCHANGED;
  menu_editing = false;
  return true;
}

bool settings_page_refresh() {
  eeprom_control_uv uv = { 0 };
  eeprom_control_get_uv(uv);

  Adafruit_SPITFT * tft = display_get_object();
  tft->setCursor(0, 0);
  tft->fillScreen(DISPLAY_BLACK);
  tft->setTextColor(DISPLAY_WHITE);
  tft->setTextSize(1);

  if (menu_actual == SETTINGS_PAGE_BACK) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  if (menu_editing) {
    tft->println("Done");
  } else {
    tft->println("Change settings");
  }

  if (menu_actual == SETTINGS_PAGE_UV_FREQ) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("UV freq: ");
  if (menu_actual == SETTINGS_PAGE_UV_FREQ && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(menu_change_value);
    tft->println(" MHz >");
  } else {
    tft->print(uv.freq);
    tft->println(" MHz");
  }

  if (menu_actual == SETTINGS_PAGE_UV_DUTY) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("UV duty: ");
  if (menu_actual == SETTINGS_PAGE_UV_DUTY && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(menu_change_value);
    tft->println(" % >");
  } else {
    tft->print(uv.duty);
    tft->println(" %");
  }

  if (menu_actual == SETTINGS_PAGE_UV_ON) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("UV: ");
  if (menu_actual == SETTINGS_PAGE_UV_ON && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(uv_control_is_on() ? "ON" : "OFF");
    tft->println(" >");
  } else {
    tft->println(uv_control_is_on() ? "ON" : "OFF");
  }

  if (menu_actual == SETTINGS_PAGE_ALRM_L1) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("Alarm Level1: ");
  if (menu_actual == SETTINGS_PAGE_ALRM_L1 && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(menu_change_value);
    tft->println(" uR >");
  } else {
    tft->print(alarm_manager_getlevel(1));
    tft->println(" uR");
  }

  if (menu_actual == SETTINGS_PAGE_ALRM_L2) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("Alarm Level2: ");
  if (menu_actual == SETTINGS_PAGE_ALRM_L2 && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(menu_change_value);
    tft->println(" uR >");
  } else {
    tft->print(alarm_manager_getlevel(2));
    tft->println(" uR");
  }

  if (menu_actual == SETTINGS_PAGE_ALRM_NI) {
    tft->print("*");
  } else {
    tft->print(" ");
  }
  tft->print("Alarm NoImpl: ");
  if (menu_actual == SETTINGS_PAGE_ALRM_NI && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    tft->print(" < ");
    tft->print(menu_change_value);
    tft->println(" sec >");
  } else {
    tft->print(alarm_manager_getlevel(ALARM_MANAGER_NI_LEVEL));
    tft->println(" sec");
  }
  return true;
}

bool settings_page_on_left() {
  if (!menu_editing) {
    return false;
  }
  
  if (menu_actual == SETTINGS_PAGE_UV_FREQ && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value -= 500;
    if (menu_change_value < 60000) {
      menu_change_value = 60000;
    }

    eeprom_control_uv uv = { 0 };
    eeprom_control_get_uv(uv);
    uv_control_change_pwm_with_testrun(menu_change_value, uv.duty);
  } else if (menu_actual == SETTINGS_PAGE_UV_DUTY && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value--;
    if (menu_change_value < 1) {
      menu_change_value = 1;
    }

    eeprom_control_uv uv = { 0 };
    eeprom_control_get_uv(uv);
    uv_control_change_pwm_with_testrun(uv.freq, menu_change_value);
  } else if (menu_actual == SETTINGS_PAGE_UV_ON && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (uv_control_is_on()) {
      uv_control_disable_pwm();
    } else {
      uv_control_enable_pwm();
    }
  } else if ((menu_actual == SETTINGS_PAGE_ALRM_L1 || menu_actual == SETTINGS_PAGE_ALRM_L2 || menu_actual == SETTINGS_PAGE_ALRM_NI) && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value--;
    if (menu_change_value < 1) {
      menu_change_value = 1;
    }
  } else {
    if (menu_actual == SETTINGS_PAGE_MINVAL) {
      menu_actual = SETTINGS_PAGE_MAXVAL;
    } else {
      menu_actual--;
    }
  }
  
  return true;
}

bool settings_page_on_right() {
  if (!menu_editing) {
    return false;
  }

  if (menu_actual == SETTINGS_PAGE_UV_FREQ && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value += 500;
    if (menu_change_value > 120000) {
      menu_change_value = 120000;
    }

    eeprom_control_uv uv = { 0 };
    eeprom_control_get_uv(uv);
    uv_control_change_pwm_with_testrun(menu_change_value, uv.duty);
  } else if (menu_actual == SETTINGS_PAGE_UV_DUTY && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value++;
    if (menu_change_value > 30) {
      menu_change_value = 30;
    }

    eeprom_control_uv uv = { 0 };
    eeprom_control_get_uv(uv);
    uv_control_change_pwm_with_testrun(uv.freq, menu_change_value);
  } else if (menu_actual == SETTINGS_PAGE_UV_ON && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    if (uv_control_is_on()) {
      uv_control_disable_pwm();
    } else {
      uv_control_enable_pwm();
    }
  } else if ((menu_actual == SETTINGS_PAGE_ALRM_L1 || menu_actual == SETTINGS_PAGE_ALRM_L2 || menu_actual == SETTINGS_PAGE_ALRM_NI) && menu_change_value != SETTINGS_PAGE_UNCHANGED) {
    menu_change_value++;
    if (menu_change_value == 0xFFFF) {
      menu_change_value = 0xFFFF;
    }
  } else {
    if (menu_actual == SETTINGS_PAGE_MAXVAL) {
      menu_actual = SETTINGS_PAGE_MINVAL;
    } else {
      menu_actual++;
    }
  }
  
  return true;
}

bool settings_page_on_click() {
  if (menu_actual == SETTINGS_PAGE_BACK) {
    menu_editing = !menu_editing;
    if (menu_editing) {
      menu_actual++;
    }
  } else if (menu_actual == SETTINGS_PAGE_UV_FREQ || menu_actual == SETTINGS_PAGE_UV_DUTY) {
    eeprom_control_uv uv = { 0 };
    eeprom_control_get_uv(uv);
    if (menu_change_value == SETTINGS_PAGE_UNCHANGED) {
      menu_change_value = menu_actual == SETTINGS_PAGE_UV_FREQ ? uv.freq : uv.duty;
    } else {
      if (menu_actual == SETTINGS_PAGE_UV_FREQ) {
        uv.freq = menu_change_value;
      } else {
        uv.duty = menu_change_value;
      }
      eeprom_control_save_uv_freq(uv);
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
  }
  
  return true;
}
