//
// Created by tibor on 02/06/2025.
//
#include <Arduino.h>
#include <CAN.h>

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4
#define BUTTON_PIN 21

volatile bool buttonPressed = false;
volatile bool buttonState = false;
volatile unsigned long lastMillis = 0;

void IRAM_ATTR onButtonChange(){
    const unsigned long currentMillis = millis();

    if (currentMillis - lastMillis > 200) {
        buttonPressed = true;
        lastMillis = currentMillis;
    }
}

void initCAN(const int rxPin, const int txPin, const long bitRate){
    CAN.setPins(rxPin, txPin);

    for (int attempts = 0 ; attempts < 5; ++attempts) {
        if (CAN.begin(bitRate)) {
            Serial.println("CAN started");
            return;
        }

        Serial.println("Starting CAN failed, retrying...");
        delay(1000);
    }

    Serial.println("CAN start up failed, restart device");
    while (true);
}

void setup() {
    Serial.begin (115200);
    while (!Serial);
    delay (1000);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonChange, CHANGE);

    initCAN(RX_GPIO_NUM, TX_GPIO_NUM, 500E3);
}

void loop() {
    if(buttonPressed) {
        buttonPressed = false;
        buttonState = !buttonState;
        constexpr char Key[8] = "status:";
        const char *Value = buttonState ? " 1.11" : " 0";


        // Serial.println("Sending CAN packet");
        CAN.beginPacket(0x12);
        CAN.write(reinterpret_cast<const uint8_t *>(Key), 8);
        CAN.beginPacket(0x12);
        CAN.write(reinterpret_cast<const uint8_t *>(Value), strlen(Value));
        CAN.endPacket();

        Serial.print("Packet sent");
    }
}