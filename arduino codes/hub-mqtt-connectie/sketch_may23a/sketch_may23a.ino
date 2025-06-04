#include <ArduinoBLE.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "192.168.178.165";  // Laptop IP
const int mqtt_port = 1883;
const char* mqtt_user = "username";
const char* mqtt_pass = "henk";

// mqtt 
// setup clients
WiFiClient espClient;
PubSubClient client(espClient);

// WiFi gegevens
const char* ssid = "robotserver";
const char* password = "henkhenk";

// BLE Device
BLEDevice pressurePeripheral;
BLECharacteristic pressureCharacteristic;
bool pressureConnected = false;

// HTTP server
WiFiServer server(8080);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  // Verbinden met WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  connectToMQTT();
  
  server.begin();
}

void loop() {
  // Keep MQTT client running (necessary for callbacks & stability)
  client.loop();
  
  float temp = random(2000, 2100) / 100.0;  //temperature
  float hum  = random(4000, 5100) / 100.0;  //humidity
  float status = 1.11;  // Added status field
  float pressure = 1000.12;
  
  delay(3000);
  publishSensorData(temp, hum, status, pressure);
}

void connectToMQTT() {
  Serial.print("Connecting to MQTT...");
  while (!client.connected()) {
    if (client.connect("portentaClient", mqtt_user, mqtt_pass)) {
      Serial.println(" connected!");
      if (client.subscribe("sensor/data")) {
        Serial.println("✓ Subscribed to sensor/data");
      } else {
        Serial.println("✗ Failed to subscribe to sensor/data");
      }
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void publishSensorData(float temp, float hum, float status, float pressure) {
  char payload[150];
  snprintf(payload, sizeof(payload), "{\"temp\": %.2f, \"hum\": %.2f, \"status\": %.2f, \"pressure\": %.2f}", temp, hum, status, pressure);
  
  Serial.print("Publishing to sensor/data: ");
  Serial.println(payload);
  Serial.print("Payload length: ");
  Serial.println(strlen(payload));
  
  bool result = client.publish("sensor/data", payload, false);
  if (result) {
    Serial.println("✓ Publish successful");
  } else {
    Serial.println("✗ Publish failed");
    Serial.print("Client state: ");
    Serial.println(client.state());
    connectToMQTT();//retry
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}