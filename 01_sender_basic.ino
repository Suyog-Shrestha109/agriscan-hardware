// AgriScan — ESP32-to-ESP32 BLE, Stage 1: basic sender
// Author: Suyog Shrestha
// Posted: 2026-01-22 17:54:34 EST, Slack #agriscan-general
// Advertises as ESP32_SENDER and notifies a ticking counter to prove the link works.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static const char* DEVICE_NAME = "ESP32_SENDER";

// Must match receiver
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

void setup() {
Serial.begin(115200);
Serial.println("Starting BLE Sender...");

BLEDevice::init(DEVICE_NAME);
BLEServer* pServer = BLEDevice::createServer();
pServer->setCallbacks(new MyServerCallbacks());

BLEService* pService = pServer->createService(SERVICE_UUID);

pCharacteristic = pService->createCharacteristic(
CHAR_UUID,
BLECharacteristic::PROPERTY_READ |
BLECharacteristic::PROPERTY_NOTIFY
);

// Required for notifications on many clients
pCharacteristic->addDescriptor(new BLE2902());
pCharacteristic->setValue("count=0");

pService->start();

BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
pAdvertising->addServiceUUID(SERVICE_UUID);
pAdvertising->setScanResponse(true);
pAdvertising->start();

Serial.println("Advertising as ESP32_SENDER...");
}

void loop() {
static uint32_t counter = 0;
counter++;

char buf[32];
snprintf(buf, sizeof(buf), "count=%lu", (unsigned long)counter);

pCharacteristic->setValue((uint8_t*)buf, strlen(buf));

if (deviceConnected) {
pCharacteristic->notify();
Serial.print("Notified: ");
} else {
Serial.print("Set (no client): ");
}
Serial.println(buf);

delay(1000);
}
