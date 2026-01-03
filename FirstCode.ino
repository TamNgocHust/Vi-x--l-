#define BLYNK_AUTH_TOKEN  "4xusCWyFI2U--XbkJhlB8zxHYWL-dzYe"
#define BLYNK_TEMPLATE_ID "TMPL6y6h6S4yk"
#define BLYNK_TEMPLATE_NAME "tuoinuocthongminh"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial

#include <WiFi.h>

#include <BlynkSimpleEsp32.h>

#include "DHT.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "TP-LINK_708E";

char pass[] = "";

BlynkTimer timer;

bool ledStripOn = false;

#define DHTPIN 4

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int soilMoisturePin = 35; // Analog pin for the soil moisture sensor

const int waterLevelPin = 34;   // Analog pin for the water level sensor

const int ledPin = 2;

// Constants for soil moisture thresholds

const int SOIL_MOISTURE_LOW = 50;  // Adjusted to match the 0-100 range

const int SOIL_MOISTURE_HIGH = 50; // Adjusted to match the 0-100 range

const int pumpPin = 5;  // Change this to the actual pin connected to the DC pump on ESP32

int pumpState = LOW;    // Initialize pumpState to LOW (off)

int pumpMode = 0;       // 0 for automatic, 1 for manual

int waterLevel = 0;     // Water level indicator value

unsigned long previousMillis = 0;

const unsigned long interval = 1000; // Interval in milliseconds
unsigned long lcdPrevMillis = 0;
const unsigned long lcdInterval = 2000; // 2 giây đổi 1 thông số
int lcdPage = 0;



float mappedSoilMoisture = 0; // Declare mappedSoilMoisture as a global variable



void setup() {

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Khoi dong...");

  digitalWrite(pumpPin,LOW);
 
  pinMode(ledPin, OUTPUT);

  pinMode(pumpPin, OUTPUT);

  dht.begin();

  Blynk.begin(auth, ssid, pass);

  Blynk.virtualWrite(V2, LOW);

  Blynk.virtualWrite(V3, "LED OFF");

  Blynk.virtualWrite(V6, LOW); // Initialize the mode button to automatic mode

  Blynk.virtualWrite(V5, LOW); // Initialize the pump button to OFF

}


BLYNK_WRITE(V5) { // This function is called when the pump button state changes (Manual Control)

  if (pumpMode == 1) { // Check if the mode is manual

    pumpState = param.asInt(); // Read the state of the pump button (0 for OFF, 1 for ON)

    digitalWrite(pumpPin, pumpState); // Turn the pump on or off based on button state

  }

}


BLYNK_WRITE(V6) { // This function is called when the mode button state changes

  pumpMode = param.asInt(); // Read the state of the mode button (0 for automatic, 1 for manual)

}

void loop() {

  Blynk.run();

  timer.run();
   

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    // Read soil moisture or water level based on the flag

    if (pumpMode == 0) {

      int soilSensorValue = analogRead(soilMoisturePin);
      Serial.printf("Doamdat:");

      Serial.println(soilSensorValue);

      mappedSoilMoisture = map(soilSensorValue, 0, 4095, 100, 0); // Update mappedSoilMoisture

      autoControlPump(mappedSoilMoisture); // Automatically control the pump based on soil moisture if in automatic mode

    }

    

    // Read water level

    waterLevel = analogRead(waterLevelPin);
    Serial.printf("Mucnuoc:");
    Serial.println(waterLevel);

    if (waterLevel < 450) {

      waterLevel = 0; // Set water level to 0 if below a threshold

    } else {

      waterLevel = map(waterLevel, 450, 3000, 0, 100); // Map water level to 0-100

    }

    

    // Control LED

    controlLED(mappedSoilMoisture);


    // Read and send sensor data

    readAndSendSensorData();

    Blynk.virtualWrite(V7, waterLevel); // Send water level to Blynk

  }
  displayLCD(); 

}

void autoControlPump(float mappedSoilMoisture) {

  if (waterLevel < 30) {

    // Water level is too low, do not turn on the pump

    pumpState = LOW;

    digitalWrite(ledPin, LOW);

    Blynk.virtualWrite(V2, HIGH);

    Blynk.setProperty(V2, "color", "#D3435C");

    

  } else if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {

    // If soil moisture is below the threshold and water level is okay, turn the pump ON

    pumpState = HIGH;

  } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {

    // If soil moisture is above the threshold, turn the pump OFF

    pumpState = LOW;

  }


  digitalWrite(pumpPin, pumpState); // Update the pump state

  if (pumpMode == 0) {

    Blynk.virtualWrite(V5, pumpState); // Update the pump button status on Blynk

  }

}


void controlLED(float mappedSoilMoisture) {

  if (mappedSoilMoisture <= SOIL_MOISTURE_LOW) {

    digitalWrite(ledPin, HIGH);

    Blynk.virtualWrite(V2, HIGH);

    Blynk.setProperty(V2, "color", "#00FF00");

    Blynk.virtualWrite(V3, "LED ON");

    Blynk.setProperty(V3, "color", "#00FF00");

  } else if (mappedSoilMoisture >= SOIL_MOISTURE_HIGH) {

    digitalWrite(ledPin, LOW);

    Blynk.virtualWrite(V2, HIGH);

    Blynk.setProperty(V2, "color", "#D3435C");

    Blynk.virtualWrite(V3, "LED OFF");

    Blynk.setProperty(V3, "color", "#D3435C");

  }

}


void readAndSendSensorData() {

  float humidity = dht.readHumidity();

  float temperature = dht.readTemperature();


  if (!isnan(humidity) && !isnan(temperature)) {

    Blynk.virtualWrite(V0, temperature);

    Blynk.virtualWrite(V1, humidity);
    Serial.printf("Nhietdo:");


    Serial.println(temperature);
    Serial.printf("Doamkhongkhi:");

    Serial.println(humidity);

  } else {

    Serial.println(F("Failed to read from DHT sensor!"));

  }


  if (pumpMode == 0) {

    Blynk.virtualWrite(V4, mappedSoilMoisture); // Update mappedSoilMoisture on Blynk

  }

}
void displayLCD() {
   float humidity = dht.readHumidity();

  float temperature = dht.readTemperature();
  unsigned long currentMillis = millis();

  if (currentMillis - lcdPrevMillis >= lcdInterval) {
    lcdPrevMillis = currentMillis;
    lcdPage++;
    if (lcdPage > 3) lcdPage = 0;

    lcd.clear();

    switch (lcdPage) {
      case 0:
        lcd.setCursor(0,0);
        lcd.print("Nhiet do:");
        lcd.setCursor(0,1);
        lcd.print(temperature);
        lcd.print(" C");
        break;

      case 1:
        lcd.setCursor(0,0);
        lcd.print("Do am dat:");
        lcd.setCursor(0,1);
        lcd.print(mappedSoilMoisture);
        lcd.print(" %");
        break;

      case 2:
        lcd.setCursor(0,0);
        lcd.print("Do am KK:");
        lcd.setCursor(0,1);
        lcd.print(humidity);
        lcd.print(" %");
        break;

      case 3:
        lcd.setCursor(0,0);
        lcd.print("Muc nuoc:");
        lcd.setCursor(0,1);
        lcd.print(waterLevel);
        lcd.print(" %");
        break;
    }
  }
}