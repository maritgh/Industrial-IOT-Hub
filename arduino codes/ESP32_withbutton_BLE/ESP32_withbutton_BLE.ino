#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Arduino_NiclaSenseEnv.h"

NiclaSenseEnv device;
#define SDA_PIN 5
#define SCL_PIN 6


#define BUTTON_PIN 3  // Adjust based on your wiring

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic;
bool ledState = false;
float temperature, humidity, NO2, O3;
int airQualityIndex;

void IRAM_ATTR buttonPressed() {
    
    ledState = !ledState;
    pCharacteristic->setValue(ledState ? "ON" : "OFF");
    pCharacteristic->notify();
    String temp = String(temperature);
    pCharacteristic->setValue(temp.c_str());
    pCharacteristic->notify();
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


    BLEDevice::init("ESP32-C6-Button");
    pServer = BLEDevice::createServer();
    
    BLEService *pService = pServer->createService(BLEUUID("76288a9b-d488-4317-bdc1-5142a41cc0dc"));
    pCharacteristic = pService->createCharacteristic(
        BLEUUID("6a46a075-bcff-4290-bb1d-6620eb93c734"),
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
}

void loop() {
    delay(5000); // Prevent excessive loop executions
    displayAllData();
}
void displayAllData() {
    // Get the temperature/humidity and air quality sensors
    TemperatureHumiditySensor& tempHumSensor = device.temperatureHumiditySensor();
    OutdoorAirQualitySensor& airQualitySensor = device.outdoorAirQualitySensor();

    // Check if both temperature/humidity and air quality sensors are enabled
    if (tempHumSensor.enabled() && airQualitySensor.enabled()) {
        // Read data from the Nicla Sense Env sensors
        temperature = tempHumSensor.temperature();
        humidity = tempHumSensor.humidity();
        NO2 = airQualitySensor.NO2();
        O3 = airQualitySensor.O3();
        airQualityIndex = airQualitySensor.airQualityIndex();

        // Print all sensor data in a single line, with AQI at the end
        Serial.print("- Temperature: ");
        Serial.print(temperature, 2);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity, 2);
        Serial.print(" %, NO2: ");
        Serial.print(NO2, 2);
        Serial.print(" ppb, O3: ");
        Serial.print(O3, 2);
        Serial.print(" ppb, Air Quality Index: ");
        Serial.println(airQualityIndex);
    } else {
        // Error message if one or more sensors are disabled
        Serial.println("- ERROR: One or more sensors are disabled!");
    }
}