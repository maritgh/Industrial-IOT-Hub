#include <ArduinoBLE.h>
#include <WiFi.h>

// BLE devices
BLEDevice ledPeripheral;
BLECharacteristic ledCharacteristic;
bool ledConnected = false;

BLEDevice buttonPeripheral;
BLECharacteristic buttonCharacteristic;
bool buttonConnected = false;

BLEDevice pressurePeripheral;
BLECharacteristic pressureCharacteristic;
bool pressureConnected = false;

// WiFi gegevens
const char* ssid = "bliep";
const char* password = "37al63mf32qj";

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

  Serial.println("\n WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  //  IP tonen

  server.begin();

  // BLE starten
  if (!BLE.begin()) {
    Serial.println(" BLE failed to start");
    while (1);
  }

  Serial.println("🔍 BLE Central - Scanning...");
  BLE.scan();
}

void loop() {
  handleHttpClient(); // WiFi server afhandeling

  // BLE verbindingen beheren
  if (!ledConnected || !buttonConnected || !pressureConnected) {
    BLEDevice peripheral = BLE.available();
    if (peripheral) {
      String name = peripheral.localName();

      if (name == "LED" && !ledConnected) {
        if (connectPeripheral(peripheral, "19b10001-e8f2-537e-4f6c-d104768a1214", ledPeripheral, ledCharacteristic)) {
          ledConnected = true;
          Serial.println(" LED device connected");
        }
      } else if (name == "BUTTON" && !buttonConnected) {
        if (connectPeripheral(peripheral, "e7f61469-a5b2-4eed-ad9f-4c06eb5ecd01", buttonPeripheral, buttonCharacteristic)) {
          buttonConnected = true;
          Serial.println("BUTTON device connected");
        }
      } else if (name == "PRESSURE" && !pressureConnected) {
        if (connectPeripheral(peripheral, "b4f142ab-edae-4a52-b90b-4d9e7edb1d10", pressurePeripheral, pressureCharacteristic)) {
          pressureConnected = true;
          Serial.println("PRESSURE device connected");
        }
      }
    }
  }

  // BLE data uitlezen
  if (ledConnected && ledPeripheral.connected() && ledCharacteristic.valueUpdated()) {
    byte value;
    ledCharacteristic.readValue(value);
    Serial.print("LED Value: ");
    Serial.println(value);
  } else if (ledConnected && !ledPeripheral.connected()) {
    Serial.println(" LED disconnected");
    ledConnected = false;
    BLE.scan();
  }

  if (buttonConnected && buttonPeripheral.connected() && buttonCharacteristic.valueUpdated()) {
    const uint8_t* rawValue = buttonCharacteristic.value();
    String value = String((char*)rawValue);
    value.remove(5);
    Serial.print("Button Temperature Value: ");
    Serial.println(value);
  } else if (buttonConnected && !buttonPeripheral.connected()) {
    Serial.println(" BUTTON disconnected");
    buttonConnected = false;
    BLE.scan();
  }

  if (pressureConnected && pressurePeripheral.connected() && pressureCharacteristic.valueUpdated()) {
    const uint8_t* rawValue = pressureCharacteristic.value();
    String value = String((char*)rawValue);
    value.remove(5);
    Serial.print("Pressure Value: ");
    Serial.println(value);
  } else if (pressureConnected && !pressurePeripheral.connected()) {
    Serial.println(" PRESSURE disconnected");
    pressureConnected = false;
    BLE.scan();
  }
}

//  HTTP POST handler
void handleHttpClient() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println(" Client connected");
    String currentLine = "";
    String body = "";
    bool isPost = false;
    bool readingBody = false;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        currentLine += c;

        // End of headers
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
      Serial.println("Data received via WiFi:");
      Serial.println(body);
    }

    // Stuur reactie
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");

    delay(1);
    client.stop();
    Serial.println(" Client disconnected");
  }
}

// BLE verbinding opzetten
bool connectPeripheral(BLEDevice& peripheral, const char* charUUID, BLEDevice& outDevice, BLECharacteristic& outChar) {
  Serial.print("🔗 Connecting to ");
  Serial.println(peripheral.localName());

  if (!peripheral.connect()) {
    Serial.println(" Failed to connect");
    return false;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println(" Attribute discovery failed");
    peripheral.disconnect();
    return false;
  }

  BLECharacteristic characteristic = peripheral.characteristic(charUUID);

  if (!characteristic) {
    Serial.println(" Characteristic not found");
    peripheral.disconnect();
    return false;
  }

  if (!characteristic.canSubscribe() || !characteristic.subscribe()) {
    Serial.println(" Failed to subscribe");
    peripheral.disconnect();
    return false;
  }

  outDevice = peripheral;
  outChar = characteristic;
  return true;
}
