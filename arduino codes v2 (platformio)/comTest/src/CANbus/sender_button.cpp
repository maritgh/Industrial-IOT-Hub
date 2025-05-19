#include <Arduino.h>
#include <CAN.h>

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4
#define BUTTON_PIN 21

void setup() {
    Serial.begin (115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    while (!Serial);
    delay (1000);

    Serial.println ("CAN Sender");

    // Set the pins
    CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        Serial.println("Starting CAN failed!");
        while (1);
    }
}

void loop() {
    if(digitalRead(BUTTON_PIN) == LOW) {
        Serial.print("Sending packet ... ");

        CAN.beginPacket(0x12);
        CAN.write('h');
        CAN.write('e');
        CAN.write('l');
        CAN.write('l');
        CAN.write('o');
        CAN.endPacket();

        Serial.println("done");

        delay(1000);
    }
}