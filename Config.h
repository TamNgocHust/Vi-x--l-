#ifndef CONFIG_H
#define CONFIG_H

// --- Cấu hình BLYNK ---
#define BLYNK_AUTH_TOKEN  "4xusCWyFI2U--XbkJhlB8zxHYWL-dzYe"
#define BLYNK_TEMPLATE_ID "TMPL6y6h6S4yk"
#define BLYNK_TEMPLATE_NAME "tuoinuocthongminh"
#define BLYNK_PRINT Serial

// --- Cấu hình WiFi ---
#define WIFI_SSID "TP-LINK_708E"
#define WIFI_PASS ""

// --- Cấu hình Chân (Pins) ---
#define DHTPIN 4
#define SOIL_PIN 35
#define WATER_LEVEL_PIN 34
#define LED_PIN 2
#define PUMP_PIN 5

// --- Cấu hình Cảm biến & Ngưỡng ---
#define DHTTYPE DHT11
#define SOIL_MOISTURE_LOW 50
#define SOIL_MOISTURE_HIGH 50
#define WATER_THRESHOLD_RAW 450 // Ngưỡng đọc thô của cảm biến nước

#endif