#ifndef CONFIG_H
#define CONFIG_H


#include <Arduino.h>
// --- BLYNK CONFIG ---
#define BLYNK_AUTH_TOKEN  "4xusCWyFI2U--XbkJhlB8zxHYWL-dzYe"
#define BLYNK_TEMPLATE_ID "TMPL6y6h6S4yk"
#define BLYNKEM_TPLATE_NAME "tuoinuocthongminh"
#define BLYNK_PRINT Serial


// --- WIFI CONFIG ---
//#define WIFI_SSID "TP-LINK_708E"
//#define WIFI_PASS "" // Nếu dùng Wokwi thì sửa thành "Wokwi-GUEST"
// --- WIFI CONFIG (Dành cho Wokwi) ---
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
// --- PINS ---
#define DHTPIN 4
#define SOIL_PIN 35
#define WATER_LEVEL_PIN 34
#define LED_PIN 2
#define PUMP_PIN 5


// --- CONSTANTS ---
#define DHTTYPE DHT11  
#define SOIL_MOISTURE_LOW 50
#define SOIL_MOISTURE_HIGH 50
#define WATER_THRESHOLD_RAW 450


// --- DATA STRUCTURE ---
struct GardenData {
  float temp;
  float hum;
  int soilPercent;
  int waterPercent;
};


#endif
