#ifndef PAGE_H_
#define PAGE_H_

#include "stdbool.h"

class IPage {
  public: 
    virtual void refresh()  = 0;
    
    virtual void init()     { };
    virtual bool on_left()  { return false; };
    virtual bool on_right() { return false; };
    virtual void on_click() { return false; };
};

#endif /* PAGE_H_ */
