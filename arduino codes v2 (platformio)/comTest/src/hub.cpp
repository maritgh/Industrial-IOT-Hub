#include <ArduinoBLE.h>
#include <WiFi.h>

// BLE Device
BLEDevice pressurePeripheral;
BLECharacteristic pressureCharacteristic;
bool pressureConnected = false;

// WiFi credentials
const char* ssid = "bliep";
const char* password = "37al63mf32qj";

// HTTP server
WiFiServer server(8080);

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
  handleHttpClient();

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
        Serial.print("[BLE] Data received: ");
        Serial.println(json);
      }
    } else {
      Serial.println("[BLE] PRESSURE disconnected");
      pressureConnected = false;
      BLE.scan();  // restart scanning
    }
  }
}