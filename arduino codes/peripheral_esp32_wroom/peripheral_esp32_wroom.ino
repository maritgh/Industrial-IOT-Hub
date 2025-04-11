#include <ArduinoBLE.h>

#define BUTTON_PIN 4
#define ledPin 2

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");  // BLE LED Service

BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

void ConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  BLE.advertise();
}

void DisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  BLE.advertise();
}
void IRAM_ATTR buttonPressed() {
  if (!switchCharacteristic.value()) {
        switchCharacteristic.writeValue(1);
        Serial.println("LED on");
        digitalWrite(ledPin, HIGH);
    } else {
        switchCharacteristic.writeValue(0);
        Serial.println("LED off");
        digitalWrite(ledPin, LOW);
    }
}


void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    // set LED pin to output mode
    pinMode(ledPin, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(BUTTON_PIN, buttonPressed, FALLING);


    // begin initialization
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");

        while (1)
            ;
    }
    BLE.setEventHandler(BLEConnected, ConnectHandler);
    BLE.setEventHandler(BLEDisconnected, DisconnectHandler);
    // set advertised local name and service UUID:
    BLE.setLocalName("LED");
    BLE.setAdvertisedService(ledService);

    // add the characteristic to the service
    ledService.addCharacteristic(switchCharacteristic);

    // add service
    BLE.addService(ledService);
    // set the initial value for the characeristic:
    switchCharacteristic.writeValue(0);

    // start advertising
    BLE.advertise();

    Serial.println("BLE LED Peripheral");
}

void loop() {
  BLE.poll();
  
}