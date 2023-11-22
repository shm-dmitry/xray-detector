#include "display_ili9341.h"

#include "Arduino.h"

#include "clock.h"
#include "avr/pgmspace.h"

#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set
#define ILI9341_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_DISPON 0x29   ///< Display ON
#define ILI9341_MADCTL_MY 0x80  ///< Bottom to top
#define ILI9341_MADCTL_MX 0x40  ///< Right to left
#define ILI9341_MADCTL_MV 0x20  ///< Reverse Mode
#define ILI9341_MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define ILI9341_MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define ILI9341_MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define ILI9341_MADCTL_MH 0x04  ///< LCD refresh right to left

static const uint8_t PROGMEM ILI9341_INIT_CMD[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on 
  0x00                                   // End of list
};

void display_ili9341_init(uint8_t dc) {
  display_spi_init(dc);

  display_spi_send_command(ILI9341_SWRESET);
  clock_delay(150);

  uint8_t cmd;
  const uint8_t *addr = ILI9341_INIT_CMD;
  while ((cmd = pgm_read_byte(addr++)) > 0) {
    uint8_t x = pgm_read_byte(addr++);
    uint8_t numArgs = x & 0x7F;
    display_spi_send_command_pgm(cmd, addr, numArgs);
    addr += numArgs;
    if (x & 0x80) {
      clock_delay(150);
    }
  }

  uint8_t madctl = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR;
  display_spi_send_command(ILI9341_MADCTL, &madctl, 1);
}

