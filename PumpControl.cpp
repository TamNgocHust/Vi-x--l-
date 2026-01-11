#include "PumpControl.h"


PumpControl::PumpControl() {
  pumpMode = 0; // Mặc định Auto
  pumpState = LOW;
}


void PumpControl::init() {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
}


void PumpControl::setMode(int mode) {
  pumpMode = mode;
}


void PumpControl::setManualState(int state) {
  if (pumpMode == 1) { // Chỉ cho phép chỉnh khi ở Manual
    pumpState = state;
    digitalWrite(PUMP_PIN, pumpState);
  }
}


bool PumpControl::process(GardenData data) {
  // Logic Tự động
  if (pumpMode == 0) {
    if (data.waterPercent < 30) {
      pumpState = LOW; // Hết nước thì ngắt
      // (Phần đổi màu LED trên Blynk sẽ để Display lo)
    } else if (data.soilPercent <= SOIL_MOISTURE_LOW) {
      pumpState = HIGH;
    } else if (data.soilPercent >= SOIL_MOISTURE_HIGH) {
      pumpState = LOW;
    }
    digitalWrite(PUMP_PIN, pumpState);
  }
 
  // Logic LED cứng
  controlLED(data.soilPercent);


  return (pumpState == HIGH); // Trả về trạng thái hiện tại
}


void PumpControl::controlLED(int soilPercent) {
  if (soilPercent <= SOIL_MOISTURE_LOW) {
    digitalWrite(LED_PIN, HIGH);
  } else if (soilPercent >= SOIL_MOISTURE_HIGH) {
    digitalWrite(LED_PIN, LOW);
  }
}
