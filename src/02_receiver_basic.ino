// AgriScan — ESP32-to-ESP32 BLE, Stage 2: basic receiver
// Author: Suyog Shrestha
// Posted: 2026-01-22 17:56:54 EST, Slack #agriscan-general
// Scans for ESP32_SENDER, connects, reads once, subscribes to notifications.

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

static const char* TARGET_NAME = "ESP32_SENDER";

// Must match sender
static BLEUUID SERVICE_UUID("12345678-1234-1234-1234-1234567890ab");
static BLEUUID CHAR_UUID ("abcdefab-1234-1234-1234-abcdefabcdef");

static BLEAdvertisedDevice* targetDevice = nullptr;
static bool doConnect = false;
static bool connected = false;

static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

static void notifyCallback(
BLERemoteCharacteristic* pBLERemoteCharacteristic,
uint8_t* pData,
size_t length,
bool isNotify
) {
Serial.print("Notify: ");
for (size_t i = 0; i < length; i++) Serial.print((char)pData[i]);
Serial.println();
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
void onResult(BLEAdvertisedDevice advertisedDevice) override {
if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_NAME) {
Serial.println("Found ESP32_SENDER!");
Serial.print("Address: ");
Serial.println(advertisedDevice.getAddress().toString().c_str());
Serial.print("RSSI: ");
Serial.println(advertisedDevice.getRSSI());

targetDevice = new BLEAdvertisedDevice(advertisedDevice);
doConnect = true;

BLEDevice::getScan()->stop();
}
}
};

bool connectToServer() {
Serial.println("Connecting to sender...");

BLEClient* pClient = BLEDevice::createClient();
if (!pClient->connect(targetDevice)) {
Serial.println("Connect failed.");
return false;
}
Serial.println("Connected!");

BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
if (pRemoteService == nullptr) {
Serial.println("Service not found.");
pClient->disconnect();
return false;
}

pRemoteCharacteristic = pRemoteService->getCharacteristic(CHAR_UUID);
if (pRemoteCharacteristic == nullptr) {
Serial.println("Characteristic not found.");
pClient->disconnect();
return false;
}

// Read once
if (pRemoteCharacteristic->canRead()) {
String val = pRemoteCharacteristic->readValue();
Serial.print("Initial read: ");
Serial.println(val);
}

// Subscribe to notifications
if (pRemoteCharacteristic->canNotify()) {
pRemoteCharacteristic->registerForNotify(notifyCallback);
Serial.println("Registered for notifications.");
} else {
Serial.println("Characteristic cannot notify.");
}

return true;
}

void setup() {
Serial.begin(115200);
Serial.println("Starting BLE Receiver...");

BLEDevice::init("ESP32_RECEIVER");

BLEScan* pScan = BLEDevice::getScan();
pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
pScan->setActiveScan(true);
pScan->setInterval(1349);
pScan->setWindow(449);

Serial.println("Scanning for ESP32_SENDER...");
pScan->start(0, false); // 0 = scan forever until stopped
}

void loop() {
if (doConnect && !connected) {
connected = connectToServer();
doConnect = false;

if (!connected) {
Serial.println("Retry scan in 2 seconds...");
delay(2000);
BLEDevice::getScan()->start(0, false);
}
}

delay(200);
}
