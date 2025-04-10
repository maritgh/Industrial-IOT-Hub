#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

bool deviceConnected = false;
bool oldValue = false;
unsigned long last_time = 0;
String temp = "0";

const int ledPin = 2;  // GPIO for LED

#define SERVICE_UUID        "e7f61469-a5b2-4eed-ad9f-4c06eb5ecd01"
#define CHARACTERISTIC_UUID "e7f61469-a5b2-4eed-ad9f-4c06eb5ecd01"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
    pServer->startAdvertising(); // restart advertising
  }
};

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  BLEDevice::init("BUTTON");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());

  

  pCharacteristic->setValue(temp.c_str());
  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("Waiting for client...");
}

void loop() {
  unsigned long current_time = millis();
  if (deviceConnected && (current_time - last_time > 10000)) {
    last_time = current_time;

    // Toggle value
    bool newValue = !oldValue;
    oldValue = newValue;

    uint8_t value = newValue ? 1 : 0;
    pCharacteristic->setValue(&value, 1);
    pCharacteristic->notify();

    digitalWrite(ledPin, newValue ? HIGH : LOW);
    Serial.println(newValue ? "LED on" : "LED off");
  }
}
