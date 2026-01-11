a. Khai báo Thư viện và Cấu hình định danh (Libraries & Credentials)
Phần này nạp các thư viện cần thiết để giao tiếp với phần cứng và định nghĩa các thông số kết nối Server Blynk.
Code:
// --- Cấu hình thông tin kết nối Blynk ---
#define BLYNK_AUTH_TOKEN  "4xusCWyFI2U--XbkJhlB8zxHYWL-dzYe" // Token bảo mật riêng của thiết bị
#define BLYNK_TEMPLATE_ID "TMPL6y6h6S4yk"                 // ID giao diện mẫu trên Blynk
#define BLYNK_TEMPLATE_NAME "tuoinuocthongminh"           // Tên dự án


// --- Khai báo các thư viện cần thiết ---
#include <Wire.h>               // Thư viện giao tiếp I2C
#include <LiquidCrystal_I2C.h>  // Thư viện điều khiển màn hình LCD qua I2C
#define BLYNK_PRINT Serial      // Cho phép in thông báo lỗi Blynk ra Serial Monitor


#include <WiFi.h>               // Thư viện kết nối WiFi cho ESP32
#include <BlynkSimpleEsp32.h>   // Thư viện Blynk dành riêng cho ESP32
#include "DHT.h"                // Thư viện cảm biến nhiệt độ độ ẩm DHT


// Khởi tạo đối tượng màn hình LCD địa chỉ 0x27, kích thước 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);


// Thông tin mạng WiFi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TP-LINK_708E";   // Tên WiFi
char pass[] = "";               // Mật khẩu WiFi (Đang để trống)
b. Khai báo Biến toàn cục và Chân kết nối (Variables & Pin Definitions)
Định nghĩa các chân phần cứng (GPIO) và các biến lưu trữ trạng thái hoạt động của hệ thống.
Code:
BlynkTimer timer;       // Khởi tạo bộ định thời của Blynk
bool ledStripOn = false;


// --- Cấu hình Cảm biến DHT11 ---
#define DHTPIN 4        // Chân Data của DHT nối với GPIO 4
#define DHTTYPE DHT11   // Định nghĩa loại cảm biến là DHT11
DHT dht(DHTPIN, DHTTYPE);


// --- Cấu hình Chân Analog và Thiết bị ---
const int soilMoisturePin = 35; // Chân đọc độ ẩm đất (Analog Input)
const int waterLevelPin = 34;   // Chân đọc mực nước (Analog Input)
const int ledPin = 2;           // Đèn LED báo trạng thái (GPIO 2)
const int pumpPin = 5;          // Chân điều khiển Relay bơm (GPIO 5)


// --- Các hằng số và biến logic điều khiển ---
const int SOIL_MOISTURE_LOW = 30;  // Ngưỡng dưới: Dưới 30% là Đất Khô -> Cần tưới
const int SOIL_MOISTURE_HIGH = 70; // Ngưỡng trên: Trên 70% là Đất Ướt -> Dừng tưới


int pumpState = LOW;    // Trạng thái hiện tại của bơm (LOW = Tắt, HIGH = Bật)
int pumpMode = 0;       // Chế độ hoạt động: 0 = Tự động (Auto), 1 = Thủ công (Manual)
int waterLevel = 0;     // Biến lưu giá trị mực nước (%)


// --- Biến quản lý thời gian (Multitasking) ---
unsigned long previousMillis = 0;
const unsigned long interval = 1000;    // Chu kỳ đọc cảm biến: 1000ms (1 giây)


unsigned long lcdPrevMillis = 0;
const unsigned long lcdInterval = 2000; // Chu kỳ chuyển trang LCD: 2000ms (2 giây)
int lcdPage = 0;                        // Biến đếm trang màn hình LCD


float mappedSoilMoisture = 0;           // Biến lưu độ ẩm đất đã quy đổi ra %
c. Hàm khởi tạo (Setup Function)
Chạy một lần duy nhất khi cấp nguồn, dùng để thiết lập môi trường hoạt động.
Code:
void setup() {
  Serial.begin(115200);   // Bật cổng Serial để debug lỗi
  
  // Khởi tạo màn hình LCD
  lcd.init();
  lcd.backlight();        // Bật đèn nền LCD
  lcd.clear();            // Xóa màn hình
  lcd.setCursor(0,0);
  lcd.print("Khoi dong..."); // Hiển thị thông báo khởi động


  // Cấu hình chế độ cho các chân GPIO
  digitalWrite(pumpPin,LOW); // Đảm bảo bơm tắt khi khởi động
  pinMode(ledPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);


  dht.begin(); // Khởi động cảm biến DHT


  // Kết nối đến Server Blynk qua WiFi
  Blynk.begin(auth, ssid, pass);


  // Đồng bộ trạng thái ban đầu lên App Blynk
  Blynk.virtualWrite(V2, LOW);
  Blynk.virtualWrite(V3, "LED OFF");
  Blynk.virtualWrite(V6, LOW); // Mặc định là chế độ Tự động
  Blynk.virtualWrite(V5, LOW); // Mặc định nút bơm là OFF
}
d. Các hàm giao tiếp với App Blynk (Blynk Callbacks)
Xử lý dữ liệu nhận được TỪ điện thoại gửi VỀ mạch ESP32.
Code:
// Hàm này được gọi khi nút nhấn Bơm (V5) trên App thay đổi trạng thái
BLYNK_WRITE(V5) { 
  if (pumpMode == 1) { // Chỉ thực thi nếu đang ở chế độ Thủ công (Manual)
    pumpState = param.asInt();        // Đọc trạng thái nút nhấn (0 hoặc 1)
    digitalWrite(pumpPin, pumpState); // Bật/Tắt bơm theo nút nhấn
  }
}


// Hàm này được gọi khi nút chuyển Chế độ (V6) thay đổi
BLYNK_WRITE(V6) { 
  pumpMode = param.asInt(); // Cập nhật biến chế độ: 0 = Auto, 1 = Manual
}
e. Vòng lặp chính (Loop Function)
Nơi điều phối toàn bộ hoạt động của hệ thống.
void loop() {
  Blynk.run(); // Duy trì kết nối với Server Blynk
  timer.run(); // Chạy bộ định thời (nếu có dùng SimpleTimer)


  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại của hệ thống


  // --- Kiểm tra chu kỳ đọc cảm biến (Mỗi 1 giây) ---
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;


    // 1. Xử lý cảm biến ĐỘ ẨM ĐẤT
    if (pumpMode == 0) { // Nếu đang ở chế độ Tự động
      int soilSensorValue = analogRead(soilMoisturePin); // Đọc giá trị Analog (0-4095)
      Serial.printf("Doamdat:");
      Serial.println(soilSensorValue);


      // Quy đổi sang %: 0 (Khô) - 4095 (Ướt) chuyển thành 100% - 0%
      // Lưu ý: Cảm biến điện dung thường: Càng ướt giá trị analog càng thấp -> Cần đảo ngược map
      mappedSoilMoisture = map(soilSensorValue, 0, 4095, 100, 0); 


      // Gọi hàm logic điều khiển bơm tự động
      autoControlPump(mappedSoilMoisture); 
    }
    
    // 2. Xử lý cảm biến MỰC NƯỚC
    waterLevel = analogRead(waterLevelPin);
    Serial.printf("Mucnuoc:");
    Serial.println(waterLevel);


    // Lọc nhiễu: Nếu giá trị < 450 coi như cạn sạch (0%)
    if (waterLevel < 450) {
      waterLevel = 0; 
    } else {
      // Quy đổi giá trị analog 450-3500 sang 0-100%
      waterLevel = map(waterLevel, 450, 3500, 0, 100); 
    }


    // 3. Điều khiển đèn LED chỉ thị & Gửi dữ liệu lên Blynk
    controlLED(mappedSoilMoisture);
    readAndSendSensorData();
    Blynk.virtualWrite(V7, waterLevel); // Gửi mực nước lên Pin V7
  }
  
  // --- Cập nhật hiển thị LCD (chạy độc lập với cảm biến) ---
  displayLCD(); 
}
f. Hàm Logic Điều khiển Tự động (Core Logic)
Đây là bộ não của hệ thống, quyết định việc bật/tắt bơm.
void autoControlPump(float mappedSoilMoisture) {
  // Ưu tiên 1: BẢO VỆ BƠM (Kiểm tra mực nước)
  if (waterLevel < 30) {
    pumpState = LOW; // Tắt bơm ngay lập tức
    digitalWrite(ledPin, LOW);
    
    // Gửi cảnh báo đỏ lên App
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#D3435C"); // Màu đỏ


  } 
  // Ưu tiên 2: TƯỚI NƯỚC (Khi đủ nước & Đất khô)
  else if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) { // Đất khô hơn ngưỡng 30%
    pumpState = HIGH; // Bật bơm
  } 
  // Ưu tiên 3: NGỪNG TƯỚI (Khi Đất đủ ẩm)
  else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) { // Đất ẩm hơn ngưỡng 70%
    pumpState = LOW;  // Tắt bơm
  }


  // Thực thi trạng thái ra phần cứng
  digitalWrite(pumpPin, pumpState); 


  // Đồng bộ trạng thái nút nhấn trên App (để người dùng thấy nút tự bật/tắt)
  if (pumpMode == 0) {
    Blynk.virtualWrite(V5, pumpState); 
  }
}
g. Hàm Giao tiếp Cảm biến & Hiển thị (Auxiliary Functions)
Các hàm phụ trợ giúp code gọn gàng hơn.
// Hàm điều khiển LED chỉ thị trạng thái độ ẩm trên App
void controlLED(float mappedSoilMoisture) {
  if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {
    // Đất khô -> LED Xanh lá (Báo hiệu hệ thống đang hoạt động/cần tưới)
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#00FF00"); // Màu xanh lá
    Blynk.virtualWrite(V3, "LED ON");
    Blynk.setProperty(V3, "color", "#00FF00");
  } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {
    // Đất đủ ẩm -> LED Đỏ (Báo hiệu dừng)
    digitalWrite(ledPin, LOW);
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#D3435C"); // Màu đỏ
    Blynk.virtualWrite(V3, "LED OFF");
    Blynk.setProperty(V3, "color", "#D3435C");
  }
}


// Hàm đọc DHT11 và gửi dữ liệu lên Blynk
void readAndSendSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();


  // Kiểm tra xem cảm biến có bị lỗi không (isnan = is not a number)
  if (!isnan(humidity) && !isnan(temperature)) {
    Blynk.virtualWrite(V0, temperature); // Gửi Nhiệt độ
    Blynk.virtualWrite(V1, humidity);    // Gửi Độ ẩm KK
    
    // In ra Serial Monitor để kiểm tra
    Serial.printf("Nhietdo:"); Serial.println(temperature);
    Serial.printf("Doamkhongkhi:"); Serial.println(humidity);
  } else {
    Serial.println(F("Failed to read from DHT sensor!"));
  }


  // Nếu đang Auto mode thì mới gửi độ ẩm đất tại đây
  if (pumpMode == 0) {
    Blynk.virtualWrite(V4, mappedSoilMoisture); 
  }
}


// Hàm hiển thị xoay vòng trên LCD (State Machine)
void displayLCD() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  unsigned long currentMillis = millis();


  // Chỉ thay đổi nội dung mỗi 2 giây (lcdInterval)
  if (currentMillis - lcdPrevMillis >= lcdInterval) {
    lcdPrevMillis = currentMillis;
    
    lcdPage++; // Tăng trang hiển thị
    if (lcdPage > 3) lcdPage = 0; // Quay về trang 0 nếu vượt quá trang 3


    lcd.clear(); // Xóa nội dung cũ


    // Dùng switch-case để chọn nội dung hiển thị
    switch (lcdPage) {
      case 0: // Trang 0: Nhiệt độ
        lcd.setCursor(0,0); lcd.print("Nhiet do:");
        lcd.setCursor(0,1); lcd.print(temperature); lcd.print(" C");
        break;
      case 1: // Trang 1: Độ ẩm đất
        lcd.setCursor(0,0); lcd.print("Do am dat:");
        lcd.setCursor(0,1); lcd.print(mappedSoilMoisture); lcd.print(" %");
        break;
      case 2: // Trang 2: Độ ẩm không khí
        lcd.setCursor(0,0); lcd.print("Do am KK:");
        lcd.setCursor(0,1); lcd.print(humidity); lcd.print(" %");
        break;
      case 3: // Trang 3: Mực nước
        lcd.setCursor(0,0); lcd.print("Muc nuoc:");
        lcd.setCursor(0,1); lcd.print(waterLevel); lcd.print(" %");
        break;
    }
  }
}
