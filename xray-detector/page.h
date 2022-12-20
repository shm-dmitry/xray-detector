#ifndef PAGE_H_
#define PAGE_H_

#include "stdint.h"
#include "Arduino.h"

#define PAGE_ACTION_FLAG_ISLEFT   0
#define PAGE_ACTION_FLAG_ISRIGHT  1
#define PAGE_ACTION_FLAG_ISCLICK  2
#define PAGE_ACTION_FLAG_REFRESH  3
#define PAGE_ACTION_FLAG_INIT     4
#define PAGE_ACTION_FLAG_NEEDRF   5

#define PAGE_ACTION_FLAG__MAXVAL  (PAGE_ACTION_FLAG_NEEDRF + 1)

typedef bool (* t_page_action)();

bool page_action_process(t_page_action * actions, uint8_t flags) {
  if (flags == 0) {
    return false;
  }

  bool result = false;
  for (uint8_t i = 0; i<PAGE_ACTION_FLAG__MAXVAL; i++) {
    if (flags & _BV(i)) {
      t_page_action action = actions[i];
      if (action != NULL) {
        if (action()) {
          result = true;
        }
      }
    }
  }

  return result;
}

#endif /* PAGE_H_ */
