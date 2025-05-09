#include <WiFi.h>
#include <HTTPClient.h>
#include "Arduino_NiclaSenseEnv.h"

#define SDA_PIN 5
#define SCL_PIN 6

// Wi-Fi credentials
const char* ssid = "bliep";  // Update with your Wi-Fi SSID
const char* password = "37al63mf32qj";  // Update with your Wi-Fi password

// Set the server URL (Portenta H7 IP address)
const char* serverURL = "http://172.20.10.11:8080/data";  // Update with the actual IP and port of your Portenta H7 server

// Initialize the NiclaSenseEnv sensor
NiclaSenseEnv device;
float temperature, humidity;
unsigned long last_time = 0;

void setup() {
  // Start serial communication
  Serial.begin(115200);
  while (!Serial);

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Wait until Wi-Fi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print IP address once connected
  Serial.println("\nConnected to WiFi");
  Serial.print("ESP32-C6 IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize NiclaSenseEnv sensor
  if (device.begin()) {
    Serial.println("- Nicla Sense Env is connected!");
    device.outdoorAirQualitySensor().setEnabled(true);
  } else {
    Serial.println("- ERROR: Nicla Sense Env device not found!");
  }
}

void loop() {
  // Only send data once every 10 seconds
  unsigned long current_time = millis();
  if (current_time - last_time > 10000) {
    last_time = current_time;

    // Read temperature and humidity
    TemperatureHumiditySensor& tempHumSensor = device.temperatureHumiditySensor();
    if (tempHumSensor.enabled()) {
      temperature = tempHumSensor.temperature();
      humidity = tempHumSensor.humidity();
    } else {
      Serial.println("- ERROR: TemperatureHumiditySensor sensor is disabled!");
      return;
    }

    // If Wi-Fi is connected, send data to server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverURL);  // Connect to Portenta H7 server
      http.addHeader("Content-Type", "application/json");

      // Create JSON payload with temperature and humidity data
      String payload = "{\"temperature\":" + String(temperature, 2) + ",\"humidity\":" + String(humidity, 2) + "}";
      
      // Send POST request
      int responseCode = http.POST(payload);

      // Print response from the server
      Serial.print("POST Response: ");
      Serial.println(responseCode);
      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }
}
