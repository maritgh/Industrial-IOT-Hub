#include <ArduinoBLE.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BMP280 bmp;

// BLE Service and Characteristic UUID
BLEService pressureService("b4f142ab-edae-4a52-b90b-4d9e7edb1d10");
BLEStringCharacteristic pressureCharacteristic("b4f142ab-edae-4a52-b90b-4d9e7edb1d10", BLERead | BLENotify, 50);

void ConnectHandler(const BLEDevice central) {
    Serial.print("[BLE] Connected to central: ");
    Serial.println(central.address());
}

void DisconnectHandler(const BLEDevice central) {
    Serial.print("[BLE] Disconnected from central: ");
    Serial.println(central.address());
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!bmp.begin(0x76)) {
        Serial.println("[SENSOR] BMP280 not found!");
        while (true);
    }

    if (!BLE.begin()) {
        Serial.println("[BLE] Starting BLE failed!");
        while (true);
    }

    BLE.setEventHandler(BLEConnected, ConnectHandler);
    BLE.setEventHandler(BLEDisconnected, DisconnectHandler);
    BLE.setLocalName("PRESSURE");
    BLE.setAdvertisedService(pressureService);

    pressureService.addCharacteristic(pressureCharacteristic);
    BLE.addService(pressureService);
    pressureCharacteristic.writeValue("{\"pressure\":0}");

    BLE.advertise();

    Serial.println("[BLE] Pressure Sensor Peripheral ready.");
}

void loop() {
    static unsigned long lastSendTime = 0;
    unsigned long currentTime = millis();

    BLE.poll();

    if (BLE.connected() && (currentTime - lastSendTime >= 10000)) {
        lastSendTime = currentTime;

        float pressure = bmp.readPressure() / 100.0F; // hPa
        String payload = String(pressure, 2);
        Serial.print("[SENSOR] Pressure: ");
        Serial.println(payload);

        pressureCharacteristic.writeValue(payload);
        Serial.println("[BLE] Pressure sent via BLE.");
    }
}
