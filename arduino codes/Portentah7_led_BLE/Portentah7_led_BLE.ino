/*
  LED Control

  This example scans for Bluetooth® Low Energy peripherals until one with the advertised service
  "19b10000-e8f2-537e-4f6c-d104768a1214" UUID is found. Once discovered and connected,
  it will remotely control the Bluetooth® Low Energy peripheral's LED, when the button is pressed or released.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  - Button with pull-up resistor connected to pin 2.

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

// variables for button
#define LED_PIN LED_BUILTIN  // Built-in LED pin

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  // configure the button pin as input
  pinMode(LED_PIN, OUTPUT);

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - LED control");

  // start scanning for peripherals
  BLE.scanForUuid("76288a9b-d488-4317-bdc1-5142a41cc0dc");
  //BLE.scanForUuid("bee531ce-6518-48b8-b5fb-6c00d0009c9c");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "ESP32-WROOM-Button" && peripheral.localName() != "ESP32-C6-Button") {
      return;
    }

    // if () {
    //   return;
    // }
    word test = 56;

    // stop scanning
    BLE.stopScan();

    //controlLed(peripheral);
    // peripheral disconnected, start scanning again
    //BLE.scanForUuid("bee531ce-6518-48b8-b5fb-6c00d0009c9c");
    controlLed(peripheral);
    BLE.scanForUuid("76288a9b-d488-4317-bdc1-5142a41cc0dc");
  }
}

void controlLed(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic ledCharacteristic = peripheral.characteristic("6a46a075-bcff-4290-bb1d-6620eb93c734");
  // BLECharacteristic button = peripheral2.characteristic("  520fd229-31ab-4e73-8741-a49f94f2a020");

  // if (!button) {
  //   Serial.println("Peripheral does not have button characteristic!");
  //   peripheral.disconnect();
  //   return;
  // }
  // if (!ledCharacteristic) {
  //   Serial.println("Peripheral does not have LED characteristic!");
  //   peripheral.disconnect();
  //   return;
  // }

  while (peripheral.connected()) {
    // while the peripheral is connected

    if (ledCharacteristic.valueUpdated()) {
      const uint8_t* rawValue = ledCharacteristic.value();
      String value = String((char*)rawValue);

      if (value == "ON") {
        digitalWrite(LED_PIN, LOW);
      } else if (value == "OFF") {
        digitalWrite(LED_PIN, HIGH);
      }

      Serial.println("Received from esp c6: " + value);
    }
    // if (peripheral2.connected()) {
    //   if (button.valueUpdated()) {
    //     const uint8_t* rawValue = button.value();
    //     String value = String((char*)rawValue);

    //     if (value == "ON") {
    //       digitalWrite(LED_PIN, LOW);
    //     } else if (value == "OFF") {
    //       digitalWrite(LED_PIN, HIGH);
    //     }


    //     Serial.println("Received from wroom: " + value);
    //   }
    // } else {
    //   Serial.println("Peripheral2 disconnected");
    //   BLE.scanForUuid("bee531ce-6518-48b8-b5fb-6c00d0009c9c");
    //   peripheral2 = BLE.available();
    // }
  }

  Serial.println("Peripheral disconnected");
}
