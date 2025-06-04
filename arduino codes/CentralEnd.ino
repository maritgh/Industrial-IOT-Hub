#include <ArduinoBLE.h>
#include <WiFi.h>
#include <PubSubClient.h>

// BLE Device
BLEDevice pressurePeripheral;
BLECharacteristic pressureCharacteristic;
bool pressureConnected = false;
bool wifiConnected = true;
//
float temp;  //temperature
float hum;  //humidity
float status = 1.11;  // Added status field
float pressure;
//mqtt
const char* mqtt_server = "172.20.10.14";  // Laptop IP
const int mqtt_port = 1883;
const char* mqtt_user = "username";
const char* mqtt_pass = "henk";
WiFiClient espClient;
PubSubClient client(espClient);

// WiFi gegevens
const char* ssid = "bliep";
const char* password = "37al63mf32qj";

// HTTP server
WiFiServer server(8080);


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


void handleHttpClient() {
  WiFiClient client = server.accept();
  if (client) {
    String currentLine = "";
    String body = "";
    bool isPost = false;
    bool readingBody = false;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        currentLine += c;

        if (currentLine.endsWith("\r\n\r\n")) {
          isPost = currentLine.indexOf("POST") >= 0;
          readingBody = true;
          currentLine = "";
        } else if (readingBody) {
          body += c;
          if (!client.available()) break;
        }
      }
    }

    if (isPost && body.length() > 0) {
      Serial.print("[WiFi] Data received: ");
      Serial.println(body);
      int commaIndex = body.indexOf(',');
      String tempStr = body.substring(0, commaIndex);
      String humStr = body.substring(commaIndex + 1);
      temp = tempStr.toFloat();
      hum = humStr.toFloat();
      publishSensorData(temp, hum, status, pressure);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");

    delay(1);
    client.stop();
  }
}

bool connectPeripheral(BLEDevice& peripheral, const char* charUUID, BLEDevice& outDevice, BLECharacteristic& outChar) {
  Serial.print("[BLE] Connecting to ");
  Serial.println(peripheral.localName());

  if (!peripheral.connect()) {
    Serial.println("[BLE] Connection failed");
    return false;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println("[BLE] Attribute discovery failed");
    peripheral.disconnect();
    return false;
  }

  BLECharacteristic characteristic = peripheral.characteristic(charUUID);
  if (!characteristic) {
    Serial.println("[BLE] Characteristic not found");
    peripheral.disconnect();
    return false;
  }

  if (!characteristic.canSubscribe() || !characteristic.subscribe()) {
    Serial.println("[BLE] Subscribe failed");
    peripheral.disconnect();
    return false;
  }

  outDevice = peripheral;
  outChar = characteristic;
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Connect to Wi-Fi
  Serial.print("[WiFi] Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected!");
  Serial.print("[WiFi] IP Address: ");
  Serial.println(WiFi.localIP());
  //mqtt
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  connectToMQTT();
  
  server.begin();

  // Start BLE
  if (!BLE.begin()) {
    Serial.println("[BLE] Failed to start");
    while (true);
  }

  Serial.println("[BLE] Central - Scanning...");
  BLE.scan();
}

void loop() {

  if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    Serial.println("[WiFi] Disconnected. Reconnecting...");
    WiFi.begin(ssid,password);
    wifiConnected = false;
  } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
    Serial.println("[WiFi] Connected!");
    wifiConnected = true;
  }
  handleHttpClient();
  //
  client.loop();

  //delay(3000);
  //publishSensorData(temp, hum, status, pressure);
  //


  if (!pressureConnected) {
    BLEDevice peripheral = BLE.available();  // Get the next found BLE device
    if (peripheral) {
      Serial.print("[BLE] Found device: ");
      Serial.println(peripheral.localName());

      if (peripheral.localName() == "PRESSURE") {
        Serial.println("[BLE] PRESSURE device found, trying to connect...");
        if (connectPeripheral(peripheral, "b4f142ab-edae-4a52-b90b-4d9e7edb1d10", pressurePeripheral, pressureCharacteristic)) {
          pressureConnected = true;
          Serial.println("[BLE] PRESSURE device connected");
        } else {
          Serial.println("[BLE] PRESSURE connection failed, continuing scan...");
          BLE.scan();  // restart scanning
        }
      }
    }
  } else {
    // Already connected, read data if updated
    if (pressurePeripheral.connected()) {
      if (pressureCharacteristic.valueUpdated()) {
        const uint8_t* rawValue = pressureCharacteristic.value();
        String json = String((char*)rawValue);
        Serial.print("[BLE] jSON Data received: ");
        Serial.println(json);
        //memcpy(&pressure, rawValue, sizeof(float));
        //Serial.print("[BLE] FLOAT Data received: ");
        //Serial.println(pressure);
        pressure = json.toFloat();
        Serial.print("[BLE] test  Data received: ");
        Serial.println(pressure);
        publishSensorData(temp, hum, status, pressure);

      }
    } else {
      Serial.println("[BLE] PRESSURE disconnected");
      pressureConnected = false;
      BLE.scan();  // restart scanning
    }
  }
}