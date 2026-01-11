#ifndef PUMPCONTROL_H
#define PUMPCONTROL_H


#include "Config.h"


class PumpControl {
  public:
    PumpControl();
    void init();
    bool process(GardenData data);
    void setMode(int mode);
    void setManualState(int state);


    // --- MỚI THÊM: Hàm này giúp Main lấy được chế độ hiện tại ---
    int getMode() { return pumpMode; }


  private:
    int pumpMode;  // 0: Auto, 1: Manual
    int pumpState; // LOW/HIGH
    void controlLED(int soilPercent);
};


#endif
