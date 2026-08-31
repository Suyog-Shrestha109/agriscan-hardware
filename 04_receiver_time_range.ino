// AgriScan — ESP32-to-ESP32 BLE, Stage 4: receiver with RSSI range estimation
// Author: Suyog Shrestha
// Posted: 2026-01-22 18:37:45 EST, Slack #agriscan-general
// Converts BLE signal strength (RSSI) into an estimated distance using the
// log-distance path-loss model:  d = 10^((txPower - rssi) / (10 * n))

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>

static const char* TARGET_NAME = "ESP32_SENDER";

// Must match the sender
static BLEUUID SERVICE_UUID("12345678-1234-1234-1234-1234567890ab");
static BLEUUID CHAR_UUID   ("abcdefab-1234-1234-1234-abcdefabcdef");

static BLEAdvertisedDevice* targetDevice = nullptr;
static bool doConnect = false;
static bool connected = false;

static BLEClient* pClient = nullptr;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

unsigned long lastRangePrint = 0;

// RSSI -> distance estimate (rough, for closer/farther)
float estimateDistanceMeters(int rssi, int txPower = -59, float n = 2.2f) {
  return powf(10.0f, (float)(txPower - rssi) / (10.0f * n));
}

// Called whenever the sender notifies (live time)
static void notifyCallback(
  BLERemoteCharacteristic* c,
  uint8_t* data,
  size_t length,
  bool isNotify
) {
  Serial.print("Notify payload: ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();
}

// Scan callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {
    if (d.haveName() && d.getName() == TARGET_NAME) {
      Serial.println("Found ESP32_SENDER!");
      Serial.print("Address: ");
      Serial.println(d.getAddress().toString().c_str());
      Serial.print("Scan RSSI: ");
      Serial.println(d.getRSSI());

      targetDevice = new BLEAdvertisedDevice(d);
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

void startScan() {
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scan->setActiveScan(true);
  scan->setInterval(1349);
  scan->setWindow(449);

  Serial.println("Scanning for ESP32_SENDER...");
  scan->start(0, false); // scan forever
}

bool connectToServer() {
  Serial.println("Connecting to sender...");

  pClient = BLEDevice::createClient();
  if (!pClient->connect(targetDevice)) {
    Serial.println("Connect failed.");
    return false;
  }
  Serial.println("Connected!");

  BLERemoteService* svc = pClient->getService(SERVICE_UUID);
  if (!svc) {
    Serial.println("Service not found.");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = svc->getCharacteristic(CHAR_UUID);
  if (!pRemoteCharacteristic) {
    Serial.println("Characteristic not found.");
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canRead()) {
    String val = pRemoteCharacteristic->readValue();
    Serial.print("Initial read: ");
    Serial.println(val);
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    Serial.println("Registered for notifications.");
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("Starting BLE Receiver (time + range)...");
  BLEDevice::init("ESP32_RECEIVER");

  startScan();
}

void loop() {
  // Connect when device is found
  if (doConnect && !connected) {
    connected = connectToServer();
    doConnect = false;

    if (!connected) {
      Serial.println("Retry scan in 2 seconds...");
      delay(2000);
      startScan();
    }
  }

  // Continuous range updates
  if (connected && pClient && pClient->isConnected()) {
    if (millis() - lastRangePrint >= 1000) {
      lastRangePrint = millis();

      int rssi = pClient->getRssi();
      float dist = estimateDistanceMeters(rssi);

      Serial.print("Range update | RSSI: ");
      Serial.print(rssi);
      Serial.print(" dBm | est dist: ");
      Serial.print(dist, 2);
      Serial.println(" m");
    }
  }

  delay(50);
}
