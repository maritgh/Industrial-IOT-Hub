#include <Arduino.h>
#include <CAN.h>

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4
#define BUTTON_PIN 21

volatile bool buttonPressed = false;
volatile unsigned long lastMillis = 0;

void IRAM_ATTR isr(){
    unsigned long currentMillis = millis();

    if (currentMillis - lastMillis > 200) {
        buttonPressed = true;
        lastMillis = currentMillis;
    }
}

void setup() {
    Serial.begin (115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, FALLING);

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
    if(buttonPressed) {
        buttonPressed = false;

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