#include <Arduino.h>
#include <HTTPClient.h>
#include <Arduino_NiclaSenseEnv.h>

#define SDA_PIN 5
#define SCL_PIN 6

const char* ssid = "bliep";
const char* password = "37al63mf32qj";

// Server URL (Portenta H7 IP address)
const char* serverURL = "http://172.20.10.11:8080/data";

NiclaSenseEnv device;
float temperature, humidity;
unsigned long last_time = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());

    if (device.begin()) {
        Serial.println("[SENSOR] Nicla Sense Env connected");
        device.outdoorAirQualitySensor().setEnabled(true);
    } else {
        Serial.println("[ERROR] Nicla Sense Env not found!");
    }
}

void loop() {
    unsigned long current_time = millis();
    if (current_time - last_time > 10000) {
        last_time = current_time;

        TemperatureHumiditySensor& tempHumSensor = device.temperatureHumiditySensor();
        if (tempHumSensor.enabled()) {
            temperature = tempHumSensor.temperature();
            humidity = tempHumSensor.humidity();
        } else {
            Serial.println("[ERROR] TemperatureHumiditySensor disabled!");
            return;
        }

        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin(serverURL);
            http.addHeader("Content-Type", "application/json");

            String payload = String(temperature, 2) + "," + String(humidity, 2);

            int responseCode = http.POST(payload);

            Serial.print("[HTTP] POST Response: ");
            Serial.println(responseCode);

            http.end();
        } else {
            Serial.println("[WiFi] Not connected");
        }
    }
}

