#include <ArduinoBLE.h>

BLEDevice ledPeripheral;
BLECharacteristic ledCharacteristic;
bool ledConnected = false;

BLEDevice buttonPeripheral;
BLECharacteristic buttonCharacteristic;
bool buttonConnected = false;

BLEDevice pressurePeripheral;
BLECharacteristic pressureCharacteristic;
bool pressureConnected = false;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  if (!BLE.begin()) {
    Serial.println("BLE failed to start");
    while (1)
      ;
  }

  Serial.println("BLE Central - Scanning for LED, BUTTON, and PRESSURE devices");

  BLE.scan();
}

void loop() {
  if (!ledConnected || !buttonConnected || !pressureConnected) {
    BLEDevice peripheral = BLE.available();

    if (peripheral) {
      String name = peripheral.localName();

      if (name == "LED" && !ledConnected) {
        if (connectPeripheral(peripheral, "19b10001-e8f2-537e-4f6c-d104768a1214", ledPeripheral, ledCharacteristic)) {
          ledConnected = true;
          Serial.println("LED device connected");
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

  if (ledConnected && ledPeripheral.connected()) {
    if (ledCharacteristic.valueUpdated()) {
      byte value;
      ledCharacteristic.readValue(value);
      Serial.print("LED Value: ");
      Serial.println(value);
    }
  } else if (ledConnected && !ledPeripheral.connected()) {
    Serial.println("LED peripheral disconnected");
    ledConnected = false;
    BLE.scan();
  }

  if (buttonConnected && buttonPeripheral.connected()) {
    if (buttonCharacteristic.valueUpdated()) {
      const uint8_t* rawValue = buttonCharacteristic.value();
      String value = String((char*)rawValue);
      value.remove(5);
      Serial.print("Button Temperature Value: ");
      Serial.println(value);
    }
  } else if (buttonConnected && !buttonPeripheral.connected()) {
    Serial.println("BUTTON peripheral disconnected");
    buttonConnected = false;
    BLE.scan();
  }

  if (pressureConnected && pressurePeripheral.connected()) {
    if (pressureCharacteristic.valueUpdated()) {
      const uint8_t* rawValue = pressureCharacteristic.value();
      String value = String((char*)rawValue);
      value.remove(5);  // optional: remove extra digits if needed
      Serial.print("Pressure Value: ");
      Serial.println(value);
    }
  } else if (pressureConnected && !pressurePeripheral.connected()) {
    Serial.println("PRESSURE peripheral disconnected");
    pressureConnected = false;
    BLE.scan();
  }
}

bool connectPeripheral(BLEDevice& peripheral, const char* charUUID, BLEDevice& outDevice, BLECharacteristic& outChar) {
  Serial.print("Connecting to ");
  Serial.println(peripheral.localName());

  if (!peripheral.connect()) {
    Serial.println("Failed to connect");
    return false;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println("Attribute discovery failed");
    peripheral.disconnect();
    return false;
  }

  BLECharacteristic characteristic = peripheral.characteristic(charUUID);

  if (!characteristic) {
    Serial.println("Characteristic not found");
    peripheral.disconnect();
    return false;
  }

  if (!characteristic.canSubscribe()) {
    Serial.println("Cannot subscribe to characteristic");
    peripheral.disconnect();
    return false;
  }

  if (!characteristic.subscribe()) {
    Serial.println("Failed to subscribe");
    peripheral.disconnect();
    return false;
  }

  outDevice = peripheral;
  outChar = characteristic;
  return true;
}
