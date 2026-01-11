#include "Config.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include "Sensors.h"
#include "PumpControl.h"
#include "Display.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ===== Objects =====
Sensors sensors;
PumpControl pump;
Display display;

// ===== Credentials =====
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

// ===== Telemetry queue (TaskControl -> TaskNet) =====
struct TelemetryPacket {
  GardenData data;
  bool pumpOn;
  int mode; // 0=Auto, 1=Manual
};
QueueHandle_t qTelemetry;

// ===== Commands (TaskNet -> TaskControl) =====
volatile int  g_modeCmd = 0;           // 0 Auto, 1 Manual
volatile int  g_manualPumpCmd = 0;     // 0 OFF, 1 ON
volatile bool g_manualPumpDirty = false;

// ===== UI cache (TaskNet) =====
static int  lastUiState = -1;          // 0=LOW_WATER, 1=PUMP_ON, 2=PUMP_OFF
static bool uiBootSynced = false;

// ===== Prototypes =====
void taskNet(void* pv);
void taskControl(void* pv);
void updateBlynkFromPacket(const TelemetryPacket& pkt);

void setup() {
  Serial.begin(115200);

  sensors.init();
  pump.init();
  display.init();

  qTelemetry = xQueueCreate(1, sizeof(TelemetryPacket)); // giữ gói mới nhất

  // Core 0: Network/Blynk
  xTaskCreatePinnedToCore(taskNet, "TaskNet", 8192, nullptr, 2, nullptr, 0);

  // Core 1: Control (sensor/relay/LCD)
  xTaskCreatePinnedToCore(taskControl, "TaskControl", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
  // Không dùng loop nữa
  vTaskDelay(pdMS_TO_TICKS(1000));
}

// ===== TaskNet (Core 0): WiFi/Blynk + gửi dữ liệu =====
void taskNet(void* pv) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(ssid, pass);

  Blynk.config(auth, "blynk.cloud", 80);

  unsigned long lastConnectAttempt = 0;
  unsigned long lastSendMs = 0;

  TelemetryPacket latestPkt{};
  bool hasPkt = false;

  for (;;) {
    // 1) Reconnect theo nhịp (tránh flapping)
    if (WiFi.status() == WL_CONNECTED) {
      if (!Blynk.connected()) {
        unsigned long now = millis();
        if (now - lastConnectAttempt >= 3000) {
          lastConnectAttempt = now;
          Blynk.connect(1500);
        }
      }
    }

    // 2) Luôn run để giữ heartbeat
    Blynk.run();

    // 3) Nhận gói telemetry mới nhất (không block)
    TelemetryPacket pkt;
    if (xQueueReceive(qTelemetry, &pkt, 0) == pdTRUE) {
      latestPkt = pkt;
      hasPkt = true;
    }

    // 4) Gửi lên Blynk theo chu kỳ 2s
    unsigned long now = millis();
    if (hasPkt && Blynk.connected() && (now - lastSendMs >= 2000)) {
      lastSendMs = now;
      updateBlynkFromPacket(latestPkt);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ===== TaskControl (Core 1): đọc cảm biến + điều khiển + LCD =====
void taskControl(void* pv) {
  int lastMode = -1;
  int lastManualCmd = -1;

  for (;;) {
    int mode = (int)g_modeCmd;

    if (mode != lastMode) {
      pump.setMode(mode);
      lastMode = mode;
    }

    if (mode == 1) { // Manual
      if (g_manualPumpDirty) {
        int cmd = (int)g_manualPumpCmd;
        if (cmd != lastManualCmd) {
          pump.setManualState(cmd);
          lastManualCmd = cmd;
        }
        g_manualPumpDirty = false;
      }
    }

    GardenData data = sensors.read();
    bool pumpOn = pump.process(data);
    display.showLCD(data);

    TelemetryPacket pkt;
    pkt.data = data;
    pkt.pumpOn = pumpOn;
    pkt.mode = mode;

    // queue size 1: overwrite để giữ gói mới nhất
    xQueueOverwrite(qTelemetry, &pkt);

    vTaskDelay(pdMS_TO_TICKS(1000)); // chu kỳ 1s
  }
}

// ===== UI update (chỉ TaskNet gọi Blynk API) =====
void updateBlynkFromPacket(const TelemetryPacket& pkt) {
  const GardenData& d = pkt.data;

  if (!uiBootSynced) {
    Blynk.virtualWrite(V2, LOW);
    Blynk.virtualWrite(V3, "BOOT");
    uiBootSynced = true;
  }

  Blynk.virtualWrite(V0, d.temp);
  Blynk.virtualWrite(V1, d.hum);
  Blynk.virtualWrite(V4, d.soilPercent);
  Blynk.virtualWrite(V7, d.waterPercent);

  if (pkt.mode == 0) {
    Blynk.virtualWrite(V5, pkt.pumpOn ? 1 : 0);
  }

  int uiState;
  if (d.waterPercent < 30) uiState = 0;
  else uiState = pkt.pumpOn ? 1 : 2;

  if (uiState == lastUiState) return;
  lastUiState = uiState;

  if (uiState == 0) {
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#D3435C");
    Blynk.virtualWrite(V3, "LOW WATER");
    Blynk.setProperty(V3, "color", "#D3435C");
  } else if (uiState == 1) {
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#00FF00");
    Blynk.virtualWrite(V3, "PUMP ON");
    Blynk.setProperty(V3, "color", "#00FF00");
  } else {
    Blynk.virtualWrite(V2, HIGH);
    Blynk.setProperty(V2, "color", "#D3435C");
    Blynk.virtualWrite(V3, "PUMP OFF");
    Blynk.setProperty(V3, "color", "#D3435C");
  }
}

// ===== Blynk handlers (chỉ set lệnh) =====
BLYNK_WRITE(V6) { // Mode Auto/Manual
  g_modeCmd = param.asInt();
}

BLYNK_WRITE(V5) { // Manual pump
  g_manualPumpCmd = param.asInt();
  g_manualPumpDirty = true;
}
