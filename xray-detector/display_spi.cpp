/**
This code based on projects:
- https://github.com/adafruit/Adafruit_ILI9341
- https://github.com/adafruit/Adafruit-GFX-Library
- https://github.com/adafruit/Adafruit-ST7735-Library
*/


#include "Arduino.h"
#include "display_spi.h"
#include "display_spi_font.h"
#include "SPI.h"

#define SPI_DEFAULT_FREQ 8000000L
#define AVR_WRITESPI(x) for (SPDR = (x); (!(SPSR & _BV(SPIF)));)
#define SPI_WRITE16(x) \
  SPI.transfer(x >> 8); \
  SPI.transfer(x);
#define SWAP_INT16_T(x, y) \
{ \
  uint16_t a = x; \
  x = y; \
  y = a; \
}
#define SPI_WRITE32(x) \
  SPI.transfer(x >> 24); \
  SPI.transfer(x >> 16); \
  SPI.transfer(x >> 8); \
  SPI.transfer(x);

uint8_t display_spi_dc;
SPISettings settings;

#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 128

#define DISPLAY_SPI_CASET 0x2A ///< Column Address Set
#define DISPLAY_SPI_PASET 0x2B ///< Page Address Set
#define DISPLAY_SPI_RAMWR 0x2C ///< Memory Write
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C

inline void display_spi_start_write();
inline void display_spi_end_write();
void display_spi_write_fill_rect_preclipped(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_spi_write_command(uint8_t cmd);
void display_spi_write_color(uint16_t color, uint16_t count);
void display_spi_write_pixel(uint16_t x, uint16_t y, uint16_t color);
void display_spi_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void display_spi_write_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void display_spi_write_char(char c);

uint16_t  display_spi_cursor_x;
uint16_t  display_spi_cursor_y;
uint16_t display_spi_cursor_color;
uint8_t  display_spi_cursor_size;

void display_spi_set_cursor(uint16_t x, uint16_t y) {
  display_spi_cursor_x = x;
  display_spi_cursor_y = y;
}

void display_spi_set_textcolor(uint16_t color) {
  display_spi_cursor_color = color;
}

void display_spi_set_textsize(uint8_t size) {
  display_spi_cursor_size = size;
}

void display_spi_init(uint8_t dc) {
  display_spi_dc = dc;
  pinMode(display_spi_dc, OUTPUT);
  digitalWrite(display_spi_dc, HIGH);

  pinMode(MOSI, OUTPUT);

  SPI.begin();

  settings = SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0);
}

void display_spi_send_command_pgm(uint8_t command, const uint8_t * data, uint8_t datasize) {
  SPI.beginTransaction(settings);
  digitalWrite(display_spi_dc, LOW);
  AVR_WRITESPI(command);
  digitalWrite(display_spi_dc, HIGH);
  for (uint8_t i = 0; i<datasize; i++) {
    AVR_WRITESPI(pgm_read_byte(data++));
  }
  SPI.endTransaction();
}

void display_spi_send_command(uint8_t command, const uint8_t * data, uint8_t datasize) {
  SPI.beginTransaction(settings);
  digitalWrite(display_spi_dc, LOW);
  AVR_WRITESPI(command);
  digitalWrite(display_spi_dc, HIGH);
  for (uint8_t i = 0; i<datasize; i++) {
    AVR_WRITESPI(data[i]);
  }
  SPI.endTransaction();
}

void display_spi_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  if (!w || !h) {
    return;
  }

  if (w > DISPLAY_WIDTH) {
    return;
  }

  if (h > DISPLAY_HEIGHT) {
    return;
  }

  if (x + w - 1 >= DISPLAY_WIDTH) {
    w = DISPLAY_WIDTH - x;
  }

  if (y + h - 1 >= DISPLAY_HEIGHT) {
    h = DISPLAY_HEIGHT - y;
  }

  display_spi_start_write();
  display_spi_write_fill_rect_preclipped(x, y, w, h, color);
  display_spi_end_write();
}

inline void display_spi_start_write() {
  SPI.beginTransaction(settings);
}

inline void display_spi_end_write() {
  SPI.endTransaction();
}

void display_spi_write_fill_rect_preclipped(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_spi_set_addr_window(x, y, w, h);
  display_spi_write_color(color, w * h);
}

void display_spi_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  uint16_t tmp = x + w - 1;

  display_spi_write_command(DISPLAY_SPI_CASET);
  SPI_WRITE16(x);
  SPI_WRITE16(tmp);

  tmp = y + h - 1;
  display_spi_write_command(DISPLAY_SPI_PASET);
  SPI_WRITE16(y);
  SPI_WRITE16(tmp);

  display_spi_write_command(DISPLAY_SPI_RAMWR);
}

void display_spi_write_command(uint8_t cmd) {
  digitalWrite(display_spi_dc, LOW);
  SPI.transfer(cmd);
  digitalWrite(display_spi_dc, HIGH);
}

void display_spi_write_color(uint16_t color, uint16_t count) {
  if (!count) {
    return;
  }

  uint8_t hi = color >> 8;
  uint8_t lo = color;

  while (count--) {
    AVR_WRITESPI(hi);
    AVR_WRITESPI(lo);
  }
}

void display_spi_draw_bitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint8_t w, uint8_t h, uint16_t color) {
  uint8_t b = 0;
  uint8_t byteWidth = (w + 7) / 8;
  
  display_spi_start_write();
  for (uint8_t j = 0; j < h; j++, y++) {
    for (uint8_t i = 0; i < w; i++) {
      if (i & 7) {
        b <<= 1;
      } else {
        b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      }

      if (b & 0x80) {
        display_spi_write_pixel(x + (uint16_t) i, y, color);
      }
    }
  }
  display_spi_end_write();
}

void display_spi_write_pixel(uint16_t x, uint16_t y, uint16_t color) {
  if ((x < DISPLAY_WIDTH) && (y < DISPLAY_HEIGHT)) {
    display_spi_set_addr_window(x, y, 1, 1);
    SPI_WRITE16(color);
  }
}

void display_spi_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  if (x0 == x1) {
    display_spi_fill_rect(x0, y0, 1, y1 - y0 + 1, color);
  } else if (y0 == y1) {
    display_spi_fill_rect(x0, y0, x1 - x0 + 1, 1, color);
  } else {
    display_spi_start_write();
    display_spi_write_line(x0, y0, x1, y1, color);
    display_spi_end_write();
  }
}

void display_spi_write_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    SWAP_INT16_T(x0, y0);
    SWAP_INT16_T(x1, y1);
  }

  if (x0 > x1) {
    SWAP_INT16_T(x0, x1);
    SWAP_INT16_T(y0, y1);
  }

  uint16_t dx = x1 - x0;
  uint16_t dy = abs(y1 - y0);

  int16_t err = dx / 2;
  uint16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      display_spi_write_pixel(y0, x0, color);
    } else {
      display_spi_write_pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void display_spi_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_spi_fill_rect(x, y, w, 1, color);
  display_spi_fill_rect(x, y + h - 1, w, 1, color);
  display_spi_fill_rect(x, y, 1, h, color);
  display_spi_fill_rect(x + w - 1, y, 1, h, color);
}

void display_spi_write_char(char c) {
  display_spi_start_write();

  for (uint8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&DISPLAY_FONT[c * 5 + i]);
    for (uint8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        if (display_spi_cursor_size == 1)
          display_spi_write_pixel(display_spi_cursor_x + i, display_spi_cursor_y + j, display_spi_cursor_color);
        else
          display_spi_write_fill_rect_preclipped(display_spi_cursor_x + i * display_spi_cursor_size, 
                                                 display_spi_cursor_y + j * display_spi_cursor_size, 
                                                 display_spi_cursor_size, 
                                                 display_spi_cursor_size, 
                                                 display_spi_cursor_color);
      }
    }
  }

  display_spi_end_write();

  display_spi_cursor_x += display_spi_cursor_size * 6;
}

void display_spi_prints(const char * text) {
  while(*text) {
    display_spi_write_char(*text);
    text++;
  }
}

void display_spi_println(const char * text) {
  display_spi_prints(text);
  display_spi_cursor_x = 0;
  display_spi_cursor_y += 8;
}

void display_spi_print8(uint8_t value) {
  display_spi_print32((uint32_t) value);
}

void display_spi_print16(uint16_t value) {
  display_spi_print32((uint32_t) value);
}

void display_spi_print32(uint32_t value) {
  char buffer[10] = {0};
  itoa(value, buffer, 10);
  display_spi_prints(buffer);
}

uint16_t display_spi_get_cursor_x() {
  return display_spi_cursor_x;
}

uint16_t display_spi_get_cursor_y() {
  return display_spi_cursor_y;
}
