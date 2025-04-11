#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Arduino_NiclaSenseEnv.h"

NiclaSenseEnv device;
#define SDA_PIN 5
#define SCL_PIN 6
#define BUTTON_PIN 3  


BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

bool deviceConnected = false;
bool oldValue = false;
unsigned long last_time = 0;
String temp = "0";
float temperature, humidity;


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

void IRAM_ATTR buttonPressed() {

    String temp = String(temperature);
    Serial.println(temperature);
    pCharacteristic->setValue(temp.c_str());
    pCharacteristic->notify();
    // String hum = String(humidity);
    // pCharacteristic->setValue(hum.c_str());
    // pCharacteristic->notify();
    Serial.println("button pressed!");

}

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN); 

  attachInterrupt(BUTTON_PIN, buttonPressed, FALLING);
  if (device.begin()) {
    Serial.println("- Nicla Sense Env is connected!");

        // Enable the outdoor air quality sensor
    device.outdoorAirQualitySensor().setEnabled(true);  
  } else {
        // Error message if the Nicla Sense Env is not found
    Serial.println("- ERROR: Nicla Sense Env device not found!");
  } 

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
    TemperatureHumiditySensor& tempHumSensor = device.temperatureHumiditySensor();
    OutdoorAirQualitySensor& airQualitySensor = device.outdoorAirQualitySensor();

  if (tempHumSensor.enabled()){
      temperature = tempHumSensor.temperature();
      humidity = tempHumSensor.humidity();
  } else {
    Serial.println("- ERROR: TemperatureHumiditySensor sensor is disabled!");

  }
    last_time = current_time;

    // Toggle value
    // bool newValue = !oldValue;
    // oldValue = newValue;

    // uint8_t value = newValue ? 1 : 0;
    // pCharacteristic->setValue(&value, 1);
    // pCharacteristic->notify();

    // digitalWrite(ledPin, newValue ? HIGH : LOW);
    // Serial.println(newValue ? "LED on" : "LED off");
  }
}
