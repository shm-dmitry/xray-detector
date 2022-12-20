#ifndef PAGE_FLASH_H_
#define PAGE_FLASH_H_

#include "page.h"
#include "stdint.h"

class FlashPage : public IPage {
  public:
    void refresh();
    bool on_left();
    bool on_right();
    void on_click();

  private:
    uint8_t percent = 0;
    bool changing = false;
};

#endif /* PAGE_FLASH_H_ */
