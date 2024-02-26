#define joyX A1
#define joyY A2
#define joySw 3
#define CE_PIN 9
#define CSN_PIN 10

#include "SPI.h"
#include "RF24.h"
#include "nRF24L01.h"

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

struct Data {
  byte coordX;
  byte coordY;
  bool isButtonPressed;
};

Data currentData;

void setup() {
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joySw, INPUT_PULLUP);

  Serial.begin(115200);

  radio.begin();

  radio.setAutoAck(false);

  radio.setDataRate(RF24_250KBPS);
  //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
  //Greater level = more consumption = longer distance

  radio.setPALevel(RF24_PA_MAX);

  radio.setPayloadSize(sizeof(currentData));

  radio.openWritingPipe(address);

  radio.stopListening();
}

int tempX;
int tempY;

void loop() {
  int X = analogRead(joyX);
  X = analogRead(joyX);
  X = analogRead(joyX);
  X = analogRead(joyX);

  int Y = analogRead(joyY);
  Y = analogRead(joyY);
  Y = analogRead(joyY);
  Y = analogRead(joyY);

  bool SW = digitalRead(joySw);

  X = map(X, 0, 1023, 0, 255);
  Y = map(Y, 0, 1023, 0, 255);

  if (tempX != X || tempY != Y) {
    tempX = X;
    tempY = Y;

    Serial.print(X);
    Serial.print("\t");
    Serial.println(Y);

    // Send data

    currentData.coordX = X;
    currentData.coordY = Y;
    currentData.isButtonPressed = !SW;

    bool report = radio.write(&currentData, sizeof(currentData));

    if (report) {
      Serial.println("OK");
    }

    delay(100);
  }
}
