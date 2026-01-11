#include "Display.h"


Display::Display() {
  lcd = new LiquidCrystal_I2C(0x27, 16, 2);
  lcdPrevMillis = 0;
  lcdPage = 0;
}


void Display::init() {
  lcd->init();
  lcd->backlight();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print("System Ready...");
}


void Display::showLCD(GardenData data) {
  unsigned long currentMillis = millis();
  if (currentMillis - lcdPrevMillis >= lcdInterval) {
    lcdPrevMillis = currentMillis;
    lcdPage++;
    if (lcdPage > 3) lcdPage = 0;


    lcd->clear();
    switch (lcdPage) {
      case 0:
        lcd->setCursor(0, 0); lcd->print("Nhiet do:");
        lcd->setCursor(0, 1); lcd->print(data.temp); lcd->print(" C");
        break;
      case 1:
        lcd->setCursor(0, 0); lcd->print("Do am dat:");
        lcd->setCursor(0, 1); lcd->print(data.soilPercent); lcd->print(" %");
        break;
      case 2:
        lcd->setCursor(0, 0); lcd->print("Do am KK:");
        lcd->setCursor(0, 1); lcd->print(data.hum); lcd->print(" %");
        break;
      case 3:
        lcd->setCursor(0, 0); lcd->print("Muc nuoc:");
        lcd->setCursor(0, 1); lcd->print(data.waterPercent); lcd->print(" %");
        break;
    }
  }
}
