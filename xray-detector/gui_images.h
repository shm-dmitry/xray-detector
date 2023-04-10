#ifndef GUI_IMAGES_H_
#define GUI_IMAGES_H_

#define IMG_XRAY_SMALL_W  8
#define IMG_XRAY_SMALL_H  8
static const uint8_t PROGMEM IMG_XRAY_SMALL[] = {0x7e, 0x3c, 0x3c, 0x18, 0x7e, 0xff, 0xe7, 0xc3};

#define IMG_FLASH_W  8
#define IMG_FLASH_H  8
static const uint8_t PROGMEM IMG_FLASH[] = {0x00, 0x03, 0x07, 0xff, 0xff, 0x07, 0x03, 0x00};

#define IMG_BAT_W  8
#define IMG_BAT_H  8
static const uint8_t PROGMEM IMG_BAT[] = {0x00, 0x00, 0xfe, 0x83, 0x83, 0xfe, 0x00, 0x00};

#define IMG_SETTINGS_W  8
#define IMG_SETTINGS_H  8
static const uint8_t PROGMEM IMG_SETTINGS[] = {0x3c, 0x5a, 0x81, 0xdb, 0xdb, 0x81, 0x5a, 0x3c};

#define IMG_ARROW_W  8
#define IMG_ARROW_H  8
static const uint8_t PROGMEM IMG_ARROW[] = {0x08, 0x0c, 0x0e, 0xff, 0xff, 0x0e, 0x0c, 0x08};

#define IMG_CIRCLE_W  8
#define IMG_CIRCLE_H  8
static const uint8_t PROGMEM IMG_CIRCLE[] = {0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0x3c};

#define IMG_CIRCLE_LOW_W  8
#define IMG_CIRCLE_LOW_H  8
static const uint8_t PROGMEM IMG_CIRCLE_LOW[] = {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00};

#define IMG_TRIANGLE_W  8
#define IMG_TRIANGLE_H  8
static const uint8_t PROGMEM IMG_TRIANGLE[] = {0x03, 0x0f, 0x3f, 0xff, 0xff, 0x3f, 0x0f, 0x03};


typedef struct t_icon {
  uint8_t w;
  uint8_t h;
  const uint8_t * img;
};

static const t_icon IMG_ICONS[4] = {
  {.w = IMG_XRAY_SMALL_W, .h = IMG_XRAY_SMALL_H, .img = IMG_XRAY_SMALL},
  {.w = IMG_FLASH_W,      .h = IMG_FLASH_H,      .img = IMG_FLASH},
  {.w = IMG_BAT_W,        .h = IMG_BAT_H,        .img = IMG_BAT},
  {.w = IMG_SETTINGS_W,   .h = IMG_SETTINGS_H,   .img = IMG_SETTINGS},
};

#endif /* GUI_IMAGES_H_ */
