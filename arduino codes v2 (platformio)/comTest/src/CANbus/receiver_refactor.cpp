//
// Created by Tibor on 26/05/2025.
//
#include <Arduino.h>
#include <CAN.h>

// #define TX_GPIO_NUM   38
// #define RX_GPIO_NUM   40

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4

void initSerial(const long baudRate = 115200){
    Serial.begin(baudRate);
    while (!Serial);
    delay(1000);
}

void initCAN(const int rxPin, const int txPin, const long bitRate){
    CAN.setPins(rxPin, txPin);

    for (int attempts = 0 ; attempts < 10; ++attempts) {
        if (CAN.begin(bitRate)) {
            Serial.println("CAN started.");
            return;
        }
        Serial.println("Starting CAN failed, retrying...");
        delay(1000);
    }

    Serial.println("CAN start up failed, restart device");
    while (true);
}

void printCANPacket(const long id, const int length, const String &packet){
    Serial.print("Packet id: 0x");
    Serial.print(id, HEX);
    Serial.print(" | Packet length: ");
    Serial.println(length);
    Serial.print("Body: ");
    Serial.println(packet);
}

String getCANPacket(){
    String packet;
    while(CAN.available() > 0){
        packet += String(static_cast<char>(CAN.read()));
    }
    return packet;
}

void handleCANPackets(){
    const int packetSize = CAN.parsePacket();
    if(!packetSize) return;
    const String packet = getCANPacket();

    //only needed when handling multiple CAN devices
    const long packetId = CAN.packetId();

    //just for debugging purposes
    printCANPacket(packetId, packetSize, packet);
}

void setup(){
    initSerial();
    initCAN(RX_GPIO_NUM, TX_GPIO_NUM, 500E3);
}

void loop(){
    handleCANPackets();
}


