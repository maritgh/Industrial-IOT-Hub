/**
 * Example of reading various sensor properties from the Nicla Sense Env's indoor air quality sensor ZMOD4410.
 * To read sulfur odor and odor intensity, the sensor mode must be changed.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "Arduino_NiclaSenseEnv.h"

NiclaSenseEnv device;
#define SDA_PIN 5
#define SCL_PIN 6

void displaySensorData(IndoorAirQualitySensor& sensor) {
    if (sensor.enabled()) {
        auto iaqMode = sensor.mode();
        Serial.print("🔧 Indoor air quality sensor mode: ");
        Serial.println(sensor.modeString());

        if (iaqMode == IndoorAirQualitySensorMode::sulfur) {
            Serial.print("👃 Sulfur odor: ");
            Serial.println(sensor.sulfurOdor());
            if (sensor.sulfurOdor()) {
                Serial.print("👃 Odor intensity: ");
                Serial.println(sensor.odorIntensity(), 2);
            }
        }

        if (iaqMode == IndoorAirQualitySensorMode::indoorAirQuality || iaqMode == IndoorAirQualitySensorMode::indoorAirQualityLowPower) {
            Serial.print("🏠 Indoor air quality value: ");
            Serial.println(sensor.airQuality(), 2);
            Serial.print("🏠 Indoor air quality: ");
            Serial.println(sensor.airQualityInterpreted());
            Serial.print("🏠 Relative indoor air quality: ");
            Serial.println(sensor.relativeAirQuality(), 2);
            Serial.print("🌬 CO2 (ppm): ");
            Serial.println(sensor.CO2(), 2);
            Serial.print("🌬 TVOC (mg/m3): ");
            Serial.println(sensor.TVOC(), 2);
            Serial.print("🍺 Ethanol (ppm): ");
            Serial.println(sensor.ethanol(), 2);
        }
    } else {
        Serial.println("🙅 Indoor air quality sensor is disabled");
    }
}


void setup() {

    Serial.begin(115200);   
    while (!Serial) {
        // Wait for Serial to be ready
    }
    Serial.println("starting up");
    Wire.begin(SDA_PIN, SCL_PIN); 
    
}

void loop() {
    if (device.begin()) {
        Serial.println("🔌 Device is connected");
        auto indoorAirQualitySensor = device.indoorAirQualitySensor();

        // Please note that it may take some time for the sensor to deliver the first data.
        indoorAirQualitySensor.setMode(IndoorAirQualitySensorMode::indoorAirQuality);
        displaySensorData(indoorAirQualitySensor);
        indoorAirQualitySensor.

        // Set indoor air quality sensor mode to sulfur odor (default is indoor air quality)
        // After switching modes, you may need to wait some time before the sensor delivers the first data.
        // indoorAirQualitySensor.setMode(IndoorAirQualitySensorMode::sulfur);
        // display_sensor_data(indoorAirQualitySensor);

        // Optionally disable the sensor
        // indoorAirQualitySensor.setEnabled(false);
    } else {
        Serial.println("🤷 Device could not be found. Please double-check the wiring.");
    }
    delay(2000);
}
