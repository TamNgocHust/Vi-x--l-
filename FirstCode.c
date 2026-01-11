#define BLYNK_AUTH_TOKEN  "4xusCWyFI2U--XbkJhlB8zxHYWL-dzYe"
#define BLYNK_TEMPLATE_ID "TMPL6y6h6S4yk"
#define BLYNK_TEMPLATE_NAME "tuoinuocthongminh"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// --- Cấu hình LCD ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Thông tin WiFi & Blynk ---
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TP-LINK_708E";
char pass[] = "";

// --- Cấu hình Chân & Biến ---
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int soilMoisturePin = 35;
const int waterLevelPin = 34;
const int ledPin = 2;
const int pumpPin = 5;

const int SOIL_MOISTURE_LOW = 30;
const int SOIL_MOISTURE_HIGH = 70;

int pumpState = LOW;
int pumpMode = 0; // 0: Auto, 1: Manual
int waterLevel = 0;
float mappedSoilMoisture = 0;
int lcdPage = 0; // Biến đếm trang LCD

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Khoi dong...");
  delay(1000); // Dùng delay ngay trong setup

  // Cấu hình chân
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  dht.begin();
  Blynk.begin(auth, ssid, pass);

  // Reset giao diện Blynk
  Blynk.virtualWrite(V2, LOW);
  Blynk.virtualWrite(V3, "LED OFF");
  Blynk.virtualWrite(V6, LOW);
  Blynk.virtualWrite(V5, LOW);
}

// --- Các hàm BLYNK WRITE ---
BLYNK_WRITE(V5) {
  if (pumpMode == 1) {
    pumpState = param.asInt();
    digitalWrite(pumpPin, pumpState);
  }
}

BLYNK_WRITE(V6) {
  pumpMode = param.asInt();
}
void loop() {
  Blynk.run();

  // 1. Đọc cảm biến
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  int soilRaw = analogRead(soilMoisturePin);
  mappedSoilMoisture = map(soilRaw, 0, 4095, 100, 0);

  int waterRaw = analogRead(waterLevelPin);
  if (waterRaw < 450) waterLevel = 0;
  else waterLevel = map(waterRaw, 450, 3500, 0, 100);

  // 2. Xử lý Logic Bơm (Auto)
  if (pumpMode == 0) {
    if (waterLevel < 30) {
      pumpState = LOW;
      Blynk.virtualWrite(V2, HIGH);
      Blynk.setProperty(V2, "color", "#D3435C"); // Đỏ
    } else if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {
      pumpState = HIGH;
    } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {
      pumpState = LOW;
    }
    digitalWrite(pumpPin, pumpState);
    Blynk.virtualWrite(V5, pumpState);
  }

  // 3. Điều khiển LED & Gửi dữ liệu Blynk
  if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#00FF00");
  } else {
    digitalWrite(ledPin, LOW);
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#D3435C");
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  if (pumpMode == 0) Blynk.virtualWrite(V4, mappedSoilMoisture);
  Blynk.virtualWrite(V7, waterLevel);

  // 4. Hiển thị LCD & DELAY 
  lcd.clear();
  lcdPage++;
  if (lcdPage > 3) lcdPage = 0;

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
   delay(2000); 
}
