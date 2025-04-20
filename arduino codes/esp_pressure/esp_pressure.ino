#include <ArduinoBLE.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BMP280 bmp; // BMP280 sensor

// BLE Service en Characteristic
BLEService pressureService("b4f142ab-edae-4a52-b90b-4d9e7edb1d10");  
BLEStringCharacteristic pressureCharacteristic("b4f142ab-edae-4a52-b90b-4d9e7edb1d10", BLERead | BLENotify, 20);

void ConnectHandler(BLEDevice central) {
  Serial.print("Connected: ");
  Serial.println(central.address());
}

void DisconnectHandler(BLEDevice central) {
  Serial.print("Disconnected: ");
  Serial.println(central.address());
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // BLE Setup
  BLE.setEventHandler(BLEConnected, ConnectHandler);
  BLE.setEventHandler(BLEDisconnected, DisconnectHandler);
  BLE.setLocalName("PRESSURE");
  BLE.setAdvertisedService(pressureService);

  pressureService.addCharacteristic(pressureCharacteristic);
  BLE.addService(pressureService);
  pressureCharacteristic.writeValue("0");

  BLE.advertise();

  Serial.println("BLE Pressure Sensor Peripheral is ready.");
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();

  BLE.poll();

  if (BLE.connected() && currentTime - lastSendTime >= 10000) {
    lastSendTime = currentTime;

    float pressure = bmp.readPressure() / 100.0F; // hPa
    String pressureStr = String(pressure, 2);

    Serial.print("Pressure: ");
    Serial.println(pressureStr);

    pressureCharacteristic.writeValue(pressureStr);
    Serial.println("Pressure sent via BLE.");
  }
}
