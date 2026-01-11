#ifndef SENSORS_H
#define SENSORS_H


#include "Config.h"
#include "DHT.h"


class Sensors {
  public:
    Sensors();
    void init();
    GardenData read(); // Hàm trả về toàn bộ dữ liệu cảm biến


  private:
    DHT* dht; // Dùng con trỏ để quản lý đối tượng DHT
};
#endif
