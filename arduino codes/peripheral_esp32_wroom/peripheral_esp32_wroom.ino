#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");  // BLE LED Service

BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
const int ledPin = 2;  // pin to use for the LED
unsigned long last_time = 0;

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

void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    // set LED pin to output mode
    pinMode(ledPin, OUTPUT);

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
  unsigned long current_time = millis();
  BLE.poll();
  if(current_time - last_time > 10000){ 
    last_time = current_time;
    //Sensor Code/Functions would go here
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
}