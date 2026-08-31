// AgriScan — CropBand sensor integration
// Author: Suyog Shrestha
// Written: ~April 2026, recovered from project group chat
//
// Reads the three sensor types on the CropBand node and prints them over serial:
//   DS18B20  1-Wire digital temperature probe
//   DHT11    humidity
//   2x       capacitive soil moisture probes on analog pins
//
// Capacitive probes were chosen over resistive (metal-to-soil) because resistive
// probes corrode and drift when left in wet soil, which rules them out for a
// sensor meant to stay buried in a field.
//
// Raw ADC values are printed rather than converted percentages — calibration and
// the soil physics live on the Hub, so improving the model never requires
// reflashing sensors already deployed.

#include <OneWire.h>
#include <DS18B20.h>
#include <DHT.h>

// ===== PIN SETUP =====
#define SOIL_1_PIN 0   // D0
#define SOIL_2_PIN 1   // D1
#define TEMP_PIN   3   // D3
#define DHT_PIN    4   // D4

#define DHT_TYPE DHT11   // change to DHT22 if needed

// ===== TEMP SENSOR =====
OneWire oneWire(TEMP_PIN);
DS18B20 tempSensor(&oneWire);

// ===== HUMIDITY SENSOR =====
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== Sensor System Starting ===");

  tempSensor.begin();
  dht.begin();
}

void loop() {
  // Read temperature
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempC();

  // Read humidity
  float humidity = dht.readHumidity();

  // Read soil moisture
  int soil1 = analogRead(SOIL_1_PIN);
  int soil2 = analogRead(SOIL_2_PIN);

  // Print everything
  Serial.println("----------------------------");

  Serial.print("Soil 1: ");
  Serial.println(soil1);

  Serial.print("Soil 2: ");
  Serial.println(soil2);

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000);
}
