#ifndef DISPLAY_H
#define DISPLAY_H


#include "Config.h"
#include <LiquidCrystal_I2C.h>


// Lưu ý: Không còn #include <BlynkSimpleEsp32.h> ở đây nữa


class Display {
  public:
    Display();
    void init();
   
    // Hàm này chỉ còn nhiệm vụ hiển thị LCD
    void showLCD(GardenData data);


  private:
    LiquidCrystal_I2C* lcd;
    unsigned long lcdPrevMillis;
    const unsigned long lcdInterval = 2000;
    int lcdPage;
};


#endif
