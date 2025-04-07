#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BUTTON_PIN 3  // Adjust based on your wiring

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic;
bool ledState = false;

void IRAM_ATTR buttonPressed() {
    
    ledState = !ledState;
    pCharacteristic->setValue(ledState ? "ON" : "OFF");
    pCharacteristic->notify();
    Serial.println("button pressed!");

}

void setup() {
    pinMode(BUTTON_PIN, INPUT);
    Serial.begin(115200);

    attachInterrupt(BUTTON_PIN, buttonPressed, FALLING);


    BLEDevice::init("ESP32-WROOM-Button");
    pServer = BLEDevice::createServer();
    
    BLEService *pService = pServer->createService(BLEUUID("bee531ce-6518-48b8-b5fb-6c00d0009c9c"));
    pCharacteristic = pService->createCharacteristic(
        BLEUUID("520fd229-31ab-4e73-8741-a49f94f2a020"),
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
}

void loop() {
    delay(5000); // Prevent excessive loop executions
}