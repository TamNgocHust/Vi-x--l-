#ifndef GARDEN_LIB_H
#define GARDEN_LIB_H

#include "Config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// Khởi tạo các đối tượng toàn cục
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

class SmartGarden {
  public:
    // Biến trạng thái
    int pumpState = LOW;
    int pumpMode = 0; // 0: Auto, 1: Manual
    int waterLevel = 0;
    float mappedSoilMoisture = 0;
    
    // Biến thời gian
    unsigned long previousMillis = 0;
    const unsigned long interval = 1000;
    unsigned long lcdPrevMillis = 0;
    const unsigned long lcdInterval = 2000;
    int lcdPage = 0;

    void setup() {
      Serial.begin(115200);
      
      // LCD Setup
      lcd.init();
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Khoi dong...");

      // Pin Setup
      pinMode(PUMP_PIN, OUTPUT);
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(PUMP_PIN, LOW);

      // Sensor & Network Setup
      dht.begin();
      Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

      // Reset Blynk UI
      Blynk.virtualWrite(V2, LOW);
      Blynk.virtualWrite(V3, "LED OFF");
      Blynk.virtualWrite(V6, LOW); // Mode Auto
      Blynk.virtualWrite(V5, LOW); // Pump OFF
    }

    void run() {
      Blynk.run();
      timer.run();
      
      unsigned long currentMillis = millis();

      // Logic chính chạy mỗi 1s (thay vì delay)
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        processSensorsAndControl();
      }

      displayLCD();
    }

    // Hàm điều khiển bơm thủ công (Gọi từ Blynk Write)
    void manualPumpControl(int value) {
      if (pumpMode == 1) {
        pumpState = value;
        digitalWrite(PUMP_PIN, pumpState);
      }
    }

    // Hàm chuyển chế độ (Gọi từ Blynk Write)
    void setMode(int value) {
      pumpMode = value;
    }

  private:
    void processSensorsAndControl() {
      // 1. Xử lý độ ẩm đất & Bơm tự động
      if (pumpMode == 0) {
        int soilSensorValue = analogRead(SOIL_PIN);
        Serial.printf("Doamdat: %d\n", soilSensorValue);
        mappedSoilMoisture = map(soilSensorValue, 0, 4095, 100, 0);
        autoControlPump();
      }

      // 2. Xử lý mực nước
      int rawWater = analogRead(WATER_LEVEL_PIN);
      Serial.printf("Mucnuoc: %d\n", rawWater);
      if (rawWater < WATER_THRESHOLD_RAW) {
        waterLevel = 0;
      } else {
        waterLevel = map(rawWater, WATER_THRESHOLD_RAW, 3000, 0, 100);
      }
      Blynk.virtualWrite(V7, waterLevel);

      // 3. Điều khiển LED & Gửi dữ liệu
      controlLED();
      readAndSendSensorData();
    }

    void autoControlPump() {
      if (waterLevel < 30) {
        pumpState = LOW;
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V2, HIGH);
        Blynk.setProperty(V2, "color", "#D3435C"); // Màu đỏ báo lỗi
      } else if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {
        pumpState = HIGH;
      } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {
        pumpState = LOW;
      }

      digitalWrite(PUMP_PIN, pumpState);
      
      // Đồng bộ trạng thái nút bấm trên Blynk khi ở chế độ Auto
      if (pumpMode == 0) {
        Blynk.virtualWrite(V5, pumpState);
      }
    }

    void controlLED() {
      if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V2, HIGH);
        Blynk.setProperty(V2, "color", "#00FF00");
        Blynk.virtualWrite(V3, "LED ON");
        Blynk.setProperty(V3, "color", "#00FF00");
      } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V2, HIGH);
        Blynk.setProperty(V2, "color", "#D3435C");
        Blynk.virtualWrite(V3, "LED OFF");
        Blynk.setProperty(V3, "color", "#D3435C");
      }
    }

    void readAndSendSensorData() {
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      if (!isnan(h) && !isnan(t)) {
        Blynk.virtualWrite(V0, t);
        Blynk.virtualWrite(V1, h);
        Serial.printf("Nhietdo: %.2f | Doamkhongkhi: %.2f\n", t, h);
      } else {
        Serial.println("Failed to read from DHT sensor!");
      }

      if (pumpMode == 0) {
        Blynk.virtualWrite(V4, mappedSoilMoisture);
      }
    }

    void displayLCD() {
      unsigned long currentMillis = millis();
      if (currentMillis - lcdPrevMillis >= lcdInterval) {
        lcdPrevMillis = currentMillis;
        lcdPage++;
        if (lcdPage > 3) lcdPage = 0;

        lcd.clear();
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        switch (lcdPage) {
          case 0:
            lcd.setCursor(0, 0); lcd.print("Nhiet do:");
            lcd.setCursor(0, 1); lcd.print(t); lcd.print(" C");
            break;
          case 1:
            lcd.setCursor(0, 0); lcd.print("Do am dat:");
            lcd.setCursor(0, 1); lcd.print(mappedSoilMoisture); lcd.print(" %");
            break;
          case 2:
            lcd.setCursor(0, 0); lcd.print("Do am KK:");
            lcd.setCursor(0, 1); lcd.print(h); lcd.print(" %");
            break;
          case 3:
            lcd.setCursor(0, 0); lcd.print("Muc nuoc:");
            lcd.setCursor(0, 1); lcd.print(waterLevel); lcd.print(" %");
            break;
        }
      }
    }
};

// Khai báo instance để Main.ino sử dụng
SmartGarden garden;

#endif