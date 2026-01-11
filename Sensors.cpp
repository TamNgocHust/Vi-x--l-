#include "Sensors.h"


Sensors::Sensors() {
  dht = new DHT(DHTPIN, DHTTYPE);
}


void Sensors::init() {
  dht->begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);
}


GardenData Sensors::read() {
  GardenData data;


  // 1. Đọc DHT
  data.temp = dht->readTemperature();
  data.hum = dht->readHumidity();


  // 2. Đọc độ ẩm đất & Map
  int soilRaw = analogRead(SOIL_PIN);
  data.soilPercent = map(soilRaw, 0, 4095, 100, 0);


  // 3. Đọc mực nước & Map
  int waterRaw = analogRead(WATER_LEVEL_PIN);
  if (waterRaw < WATER_THRESHOLD_RAW) {
    data.waterPercent = 0;
  } else {
    data.waterPercent = map(waterRaw, WATER_THRESHOLD_RAW, 3000, 0, 100);
  }
  // Debug nhanh
  if (isnan(data.temp) || isnan(data.hum)) {
    Serial.println("Lỗi đọc DHT!");
  }
  return data; // Trả về gói dữ liệu
}
