// --- QUAN TRỌNG: Config.h phải nằm đầu tiên ---
#include "Config.h"


#include <WiFi.h>
#include <BlynkSimpleEsp32.h> // Chỉ include thư viện này ở Main
#include "Sensors.h"
#include "PumpControl.h"
#include "Display.h"


// Khởi tạo module
Sensors sensors;
PumpControl pump;
Display display;
BlynkTimer timer;


// Biến cấu hình Blynk/WiFi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;


// --- HÀM GỬI DỮ LIỆU LÊN BLYNK (Chuyển từ Display sang đây) ---
void updateBlynk(GardenData data, bool isPumpOn, int pumpMode) {
  // 1. Gửi số liệu cảm biến
  Blynk.virtualWrite(V0, data.temp);
  Blynk.virtualWrite(V1, data.hum);
  Blynk.virtualWrite(V4, data.soilPercent);
  Blynk.virtualWrite(V7, data.waterPercent);


  // 2. Đồng bộ trạng thái nút bơm (Chỉ khi Auto)
  if (pumpMode == 0) {
    Blynk.virtualWrite(V5, isPumpOn ? 1 : 0);
  }


  // 3. Cập nhật LED ảo trên App
  if (data.waterPercent < 30) {
     Blynk.virtualWrite(V2, HIGH);
     Blynk.setProperty(V2, "color", "#D3435C"); // Đỏ (Cảnh báo nước)
  } else {
     if (isPumpOn) {
        Blynk.virtualWrite(V2, HIGH);
        Blynk.setProperty(V2, "color", "#00FF00"); // Xanh (Đang bơm)
        Blynk.virtualWrite(V3, "LED ON");
        Blynk.setProperty(V3, "color", "#00FF00");
     } else {
        Blynk.virtualWrite(V2, HIGH);
        Blynk.setProperty(V2, "color", "#D3435C"); // Đỏ (Tắt)
        Blynk.virtualWrite(V3, "LED OFF");
        Blynk.setProperty(V3, "color", "#D3435C");
     }
  }
}


// --- HÀM TIMER CHÍNH (Chạy mỗi 1s) ---
void myTimerEvent() {
  // B1: Đọc cảm biến
  GardenData data = sensors.read();


  // B2: Xử lý bơm & Lấy trạng thái
  bool isPumpOn = pump.process(data);
  int currentMode = pump.getMode();


  // B3: Hiển thị (Chia việc ra)
  display.showLCD(data);                         // Màn hình LCD tại chỗ
  updateBlynk(data, isPumpOn, currentMode);      // Gửi lên App Blynk
}


void setup() {
  Serial.begin(115200);


  sensors.init();
  pump.init();
 
  // Kết nối Blynk
  Blynk.begin(auth, ssid, pass);
 
  // Reset giao diện App khi khởi động
  Blynk.virtualWrite(V2, LOW);
  Blynk.virtualWrite(V3, "LED OFF");


  display.init();


  timer.setInterval(1000L, myTimerEvent);
}
void loop() {
  Blynk.run();
  timer.run();
}


// --- CÁC HÀM NHẬN LỆNH TỪ BLYNK ---


BLYNK_WRITE(V5) { // Nút Bơm
  pump.setManualState(param.asInt());
}


BLYNK_WRITE(V6) { // Nút Chế độ
  pump.setMode(param.asInt());
}


