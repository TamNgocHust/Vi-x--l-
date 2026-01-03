// Chỉ cần include thư viện chúng ta vừa tạo
#include "GardenLib.h"

void setup() {
  // Gọi hàm setup từ thư viện
  garden.setup();
}

void loop() {
  // Gọi hàm loop từ thư viện
  garden.run();
}

// --- Các hàm BLYNK WRITE bắt buộc phải ở file .ino ---

// Điều khiển bơm thủ công (V5)
BLYNK_WRITE(V5) {
  garden.manualPumpControl(param.asInt());
}

// Chuyển chế độ Auto/Manual (V6)
BLYNK_WRITE(V6) {
  garden.setMode(param.asInt());
}