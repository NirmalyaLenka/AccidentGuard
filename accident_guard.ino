/*
  AccidentGuard - ESP32 Accident Detection Device
  Hardware: ESP32, ADXL345, NEO-6M GPS, Touch Sensor, Buzzer, LED
  Features: Accident detection, GPS tracking, BLE transmission, Anti-theft
*/

#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ---- Pin Definitions ----
#define BUZZER_PIN        26
#define LED_PIN           27
#define TOUCH_PIN         T0   // GPIO4 - Capacitive touch
#define GPS_RX_PIN        16
#define GPS_TX_PIN        17
#define SDA_PIN           21
#define SCL_PIN           22

// ---- BLE UUIDs ----
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ---- Thresholds ----
#define ACCIDENT_THRESHOLD     2.5    // G-force threshold for accident detection
#define TOUCH_CANCEL_TAPS      3      // Number of taps to cancel alert
#define CANCEL_TAP_WINDOW      3000   // ms window for 3 taps
#define ALERT_WAIT_TIME        60000  // 60 seconds before calling emergency
#define ANTITHEFT_THRESHOLD    0.8    // G-force threshold for anti-theft movement
#define TOUCH_THRESHOLD        40     // Capacitive touch threshold value

// ---- State Machine ----
enum DeviceState {
  STATE_IDLE,
  STATE_ACCIDENT_DETECTED,
  STATE_WAITING_CANCEL,
  STATE_EMERGENCY,
  STATE_ANTITHEFT_ALERT
};

// ---- Globals ----
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
TinyGPSPlus gps;
HardwareSerial GPSSerial(2);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

DeviceState currentState = STATE_IDLE;
bool antitheftMode = false;

unsigned long accidentTime = 0;
unsigned long lastTapTime = 0;
int cancelTapCount = 0;

float currentLat = 0.0;
float currentLng = 0.0;
bool gpsValid = false;

// ---- BLE Callbacks ----
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client disconnected");
    BLEDevice::startAdvertising();
  }
};

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  Serial.println("AccidentGuard starting...");

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!accel.begin()) {
    Serial.println("ERROR: ADXL345 not found. Check wiring.");
    while (1) { blinkLED(100); }
  }
  accel.setRange(ADXL345_RANGE_16_G);
  accel.setDataRate(ADXL345_DATARATE_100_HZ);
  Serial.println("ADXL345 initialized");

  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS UART initialized");

  setupBLE();
  Serial.println("BLE initialized");

  blinkLED(500);
  blinkLED(500);
  Serial.println("AccidentGuard ready.");
}

// ---- Main Loop ----
void loop() {
  readGPS();
  detectAccident();
  handleTouchInput();
  handleStateMachine();
  transmitBLE();
  delay(50);
}

// ---- GPS Reading ----
void readGPS() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }
  if (gps.location.isValid()) {
    currentLat = gps.location.lat();
    currentLng = gps.location.lng();
    gpsValid = true;
  }
}

// ---- Accident Detection ----
void detectAccident() {
  if (currentState != STATE_IDLE && currentState != STATE_ANTITHEFT_ALERT) return;

  sensors_event_t event;
  accel.getEvent(&event);

  float totalG = sqrt(
    pow(event.acceleration.x, 2) +
    pow(event.acceleration.y, 2) +
    pow(event.acceleration.z, 2)
  ) / 9.81;

  if (antitheftMode && currentState == STATE_IDLE) {
    if (totalG > ANTITHEFT_THRESHOLD) {
      Serial.println("Anti-theft triggered!");
      currentState = STATE_ANTITHEFT_ALERT;
      accidentTime = millis();
    }
    return;
  }

  if (!antitheftMode && totalG > ACCIDENT_THRESHOLD) {
    Serial.print("Accident detected! G-force: ");
    Serial.println(totalG);
    currentState = STATE_ACCIDENT_DETECTED;
    accidentTime = millis();
    cancelTapCount = 0;
  }
}

// ---- Touch Input Handling ----
void handleTouchInput() {
  int touchValue = touchRead(TOUCH_PIN);
  bool touched = (touchValue < TOUCH_THRESHOLD);

  static bool lastTouched = false;
  if (touched && !lastTouched) {
    unsigned long now = millis();

    if (now - lastTapTime > CANCEL_TAP_WINDOW) {
      cancelTapCount = 0;
    }

    cancelTapCount++;
    lastTapTime = now;

    Serial.print("Touch tap count: ");
    Serial.println(cancelTapCount);

    if (cancelTapCount >= TOUCH_CANCEL_TAPS) {
      cancelAlert();
      cancelTapCount = 0;
    }

    // Toggle anti-theft: 5 quick taps in idle
    if (currentState == STATE_IDLE && cancelTapCount == 5) {
      antitheftMode = !antitheftMode;
      cancelTapCount = 0;
      Serial.print("Anti-theft mode: ");
      Serial.println(antitheftMode ? "ON" : "OFF");
      if (antitheftMode) {
        beep(200); beep(200);
      } else {
        beep(500);
      }
    }
  }
  lastTouched = touched;
}

// ---- Cancel Alert ----
void cancelAlert() {
  if (currentState == STATE_ACCIDENT_DETECTED ||
      currentState == STATE_WAITING_CANCEL ||
      currentState == STATE_ANTITHEFT_ALERT) {
    Serial.println("Alert cancelled by user (3 taps)");
    currentState = STATE_IDLE;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

// ---- State Machine Handler ----
void handleStateMachine() {
  unsigned long elapsed = millis() - accidentTime;

  switch (currentState) {
    case STATE_IDLE:
      digitalWrite(LED_PIN, LOW);
      break;

    case STATE_ACCIDENT_DETECTED:
      // Flash LED and buzz to alert user
      blinkLED(200);
      beep(100);
      Serial.println("Accident confirmed. Waiting for user cancel (60s)...");
      currentState = STATE_WAITING_CANCEL;
      accidentTime = millis();
      break;

    case STATE_WAITING_CANCEL:
      // Pulse buzzer and flash LED every second
      if (elapsed % 1000 < 100) {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
      }

      if (elapsed >= ALERT_WAIT_TIME) {
        Serial.println("No cancel received. Triggering emergency call!");
        currentState = STATE_EMERGENCY;
        accidentTime = millis();
      }
      break;

    case STATE_EMERGENCY:
      // Continuous high buzzer + fast LED flash
      digitalWrite(BUZZER_PIN, HIGH);
      if (elapsed % 200 < 100) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }

      // Buzzer runs for 1 minute then goes off but LED stays
      if (elapsed >= 60000) {
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("Emergency buzzer stopped after 1 min.");
      }

      sendEmergencySignal();
      break;

    case STATE_ANTITHEFT_ALERT:
      // Rapid buzzer + LED for antitheft
      if (elapsed % 500 < 250) {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
      }
      if (elapsed >= 60000) {
        currentState = STATE_IDLE;
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
      }
      break;
  }
}

// ---- Emergency Signal ----
void sendEmergencySignal() {
  // BLE broadcast is handled in transmitBLE()
  // SMS/Call would require SIM800L module - placeholder logic
  Serial.println("=== EMERGENCY SIGNAL ===");
  Serial.print("GPS: ");
  Serial.print(currentLat, 6);
  Serial.print(", ");
  Serial.println(currentLng, 6);
  Serial.println("Send to ambulance and emergency contacts via BLE/App");
}

// ---- BLE Transmission ----
void transmitBLE() {
  if (!deviceConnected) return;

  static unsigned long lastTransmit = 0;
  if (millis() - lastTransmit < 1000) return;
  lastTransmit = millis();

  String payload = buildPayload();
  pCharacteristic->setValue(payload.c_str());
  pCharacteristic->notify();
  Serial.print("BLE TX: ");
  Serial.println(payload);
}

// ---- Build JSON Payload ----
String buildPayload() {
  String state = "IDLE";
  if (currentState == STATE_WAITING_CANCEL) state = "ACCIDENT_WAITING";
  if (currentState == STATE_EMERGENCY) state = "EMERGENCY";
  if (currentState == STATE_ANTITHEFT_ALERT) state = "ANTITHEFT";

  String json = "{";
  json += "\"state\":\"" + state + "\",";
  json += "\"lat\":" + String(currentLat, 6) + ",";
  json += "\"lng\":" + String(currentLng, 6) + ",";
  json += "\"gps_valid\":" + String(gpsValid ? "true" : "false") + ",";
  json += "\"antitheft\":" + String(antitheftMode ? "true" : "false") + ",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  return json;
}

// ---- BLE Setup ----
void setupBLE() {
  BLEDevice::init("AccidentGuard");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("AccidentGuard Ready");
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

// ---- Helpers ----
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
}

void blinkLED(int duration) {
  digitalWrite(LED_PIN, HIGH);
  delay(duration);
  digitalWrite(LED_PIN, LOW);
  delay(duration);
}
