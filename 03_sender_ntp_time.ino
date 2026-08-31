// AgriScan — ESP32-to-ESP32 BLE, Stage 3: sender with real-time clock
// Author: Suyog Shrestha
// Posted: 2026-01-22 18:33:48 EST, Slack #agriscan-general
// Pulls time over WiFi via NTP, with graceful fallback to device uptime
// if WiFi is unavailable. Streams timestamped payloads over BLE.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <WiFi.h>
#include <time.h>

// ====== SET THESE FOR REAL CLOCK TIME (NTP) ======
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* NTP_SERVER = "pool.ntp.org";

// New York offset (EST). Change if needed.
const long GMT_OFFSET_SEC = -5 * 3600;
const int DST_OFFSET_SEC = 0; // set 3600 if you want DST manually

// ===== BLE IDs (must match receiver) =====
static const char* DEVICE_NAME = "ESP32_SENDER";
static BLEUUID SERVICE_UUID("12345678-1234-1234-1234-1234567890ab");
static BLEUUID CHAR_UUID ("abcdefab-1234-1234-1234-abcdefabcdef");

BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
void onConnect(BLEServer* pServer) override {
deviceConnected = true;
Serial.println("Client connected!");
}
void onDisconnect(BLEServer* pServer) override {
deviceConnected = false;
Serial.println("Client disconnected. Restarting advertising...");
BLEDevice::startAdvertising();
}
};

void setupTimeViaNTP() {
WiFi.mode(WIFI_STA);
WiFi.begin(WIFI_SSID, WIFI_PASS);

Serial.print("WiFi connecting");
unsigned long start = millis();
while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
delay(250);
Serial.print(".");
}
Serial.println();

if (WiFi.status() == WL_CONNECTED) {
Serial.print("WiFi OK: ");
Serial.println(WiFi.localIP());

configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);

// wait briefly for NTP to sync
for (int i = 0; i < 20; i++) {
struct tm t;
if (getLocalTime(&t, 50)) {
Serial.println("Time synced via NTP.");
return;
}
delay(200);
}
Serial.println("NTP not ready yet; will fallback to uptime until it is.");
} else {
Serial.println("WiFi failed; will use uptime time.");
}
}

String getTimeString() {
struct tm timeinfo;
if (getLocalTime(&timeinfo, 50)) {
char buf[32];
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
return String(buf);
}
return String("uptime_ms=") + String(millis());
}

void setup() {
Serial.begin(115200);
delay(300);

Serial.println("Starting BLE Sender (live time only)...");
setupTimeViaNTP();

BLEDevice::init(DEVICE_NAME);

BLEServer* pServer = BLEDevice::createServer();
pServer->setCallbacks(new MyServerCallbacks());

BLEService* pService = pServer->createService(SERVICE_UUID);

pCharacteristic = pService->createCharacteristic(
CHAR_UUID,
BLECharacteristic::PROPERTY_READ |
BLECharacteristic::PROPERTY_NOTIFY
);
pCharacteristic->addDescriptor(new BLE2902());
pCharacteristic->setValue("time=booting...");

pService->start();

BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
pAdvertising->addServiceUUID(SERVICE_UUID);
pAdvertising->setScanResponse(true);
pAdvertising->start();

Serial.println("Advertising as ESP32_SENDER...");
}

void loop() {
String payload = "time=" + getTimeString();

pCharacteristic->setValue(payload.c_str());

if (deviceConnected) {
pCharacteristic->notify();
Serial.print("Notified: ");
} else {
Serial.print("Set (no client): ");
}
Serial.println(payload);

delay(1000);
}
