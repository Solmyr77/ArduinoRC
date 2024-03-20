#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftPWM.h>

struct Data {
  byte coordX = 128;
  byte coordY = 127;
  bool isButtonPressed = false;
};

RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";

Data currentData;

#define LED_PIN A4

#define FL_MOTOR_PIN_1 2
#define FL_MOTOR_PIN_2 3

#define FR_MOTOR_PIN_1 6
#define FR_MOTOR_PIN_2 7

#define RL_MOTOR_PIN_1 4
#define RL_MOTOR_PIN_2 5

#define RR_MOTOR_PIN_1 A0
#define RR_MOTOR_PIN_2 A1

#define FL 0
#define FR 1
#define RL 2
#define RR 3

#define FORWARD 0
#define BACKWARD 1

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  SoftPWMBegin();

  Serial.begin(115200);

  initRadio();

  digitalWrite(LED_PIN, HIGH);
}

void initRadio() {
  radio.begin();
  //Append ACK packet from the receiving radio back to the transmitting radio
  radio.setAutoAck(true); //(true|false)
  //Set the transmission datarate
  radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
  //Greater level = more consumption = longer distance
  radio.setPALevel(RF24_PA_MAX); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX)
  //Default value is the maximum 32 bytes1
  radio.setPayloadSize(sizeof(currentData));
  //Act as receiver
  radio.openReadingPipe(0, address);

  radio.startListening();
}

void ChangeMotorSpeed(int motor, int motorSpeed, int motorDirection) {
  if (motorDirection == FORWARD) {
    switch (motor) {
      case FL:
        SoftPWMSet(FL_MOTOR_PIN_1, 0);
        SoftPWMSet(FL_MOTOR_PIN_2, motorSpeed);
        break;

      case FR:
        SoftPWMSet(FR_MOTOR_PIN_1, motorSpeed);
        SoftPWMSet(FR_MOTOR_PIN_2, 0);
        break;

      case RL:
        SoftPWMSet(RL_MOTOR_PIN_1, 0);
        SoftPWMSet(RL_MOTOR_PIN_2, motorSpeed);
        break;

      case RR:
        SoftPWMSet(RR_MOTOR_PIN_1, 0);
        SoftPWMSet(RR_MOTOR_PIN_2, motorSpeed);
        break;
    }
  }

  else {
    switch (motor) {
      case FL:
        SoftPWMSet(FL_MOTOR_PIN_1, motorSpeed);
        SoftPWMSet(FL_MOTOR_PIN_2, 0);
        break;

      case FR:
        SoftPWMSet(FR_MOTOR_PIN_1, 0);
        SoftPWMSet(FR_MOTOR_PIN_2, motorSpeed);
        break;

      case RL:
        SoftPWMSet(RL_MOTOR_PIN_1, motorSpeed);
        SoftPWMSet(RL_MOTOR_PIN_2, 0);
        break;

      case RR:
        SoftPWMSet(RR_MOTOR_PIN_1, motorSpeed);
        SoftPWMSet(RR_MOTOR_PIN_2, 0);
        break;
    }
  }
}

void SoftStopMotors() {
  SoftPWMSet(FL_MOTOR_PIN_1, 0);
  SoftPWMSet(FL_MOTOR_PIN_2, 0);

  SoftPWMSet(RL_MOTOR_PIN_1, 0);
  SoftPWMSet(RL_MOTOR_PIN_2, 0);

  SoftPWMSet(FR_MOTOR_PIN_1, 0);
  SoftPWMSet(FR_MOTOR_PIN_2, 0);

  SoftPWMSet(RR_MOTOR_PIN_1, 0);
  SoftPWMSet(RR_MOTOR_PIN_2, 0);
}

void ControlLogic(Data currentData) {
  currentData.coordX = map(currentData.coordX, 0, 255, 255, 0);
  currentData.coordY = map(currentData.coordY, 0, 255, 255, 0);

  if (currentData.coordX > 125 && currentData.coordX < 132) {
    if (currentData.coordY > 124 && currentData.coordY < 130) {
      SoftStopMotors();
      return 0;
    }
  }

  if (currentData.coordX >= 132) {
    int mappedX = map(currentData.coordX, 128, 255, 0, 255);

    ChangeMotorSpeed(FL, mappedX, FORWARD);
    ChangeMotorSpeed(FR, mappedX, FORWARD);
    ChangeMotorSpeed(RL, mappedX, FORWARD);
    ChangeMotorSpeed(RR, mappedX, FORWARD);
  }

  else if (currentData.coordX <= 125) {
    int mappedX = map(currentData.coordX, 0, 128, 255, 0);

    ChangeMotorSpeed(FL, mappedX, BACKWARD);
    ChangeMotorSpeed(FR, mappedX, BACKWARD);
    ChangeMotorSpeed(RL, mappedX, BACKWARD);
    ChangeMotorSpeed(RR, mappedX, BACKWARD);
  }

  else if (currentData.coordY >= 130) {
    int mappedX = map(currentData.coordX, 128, 255, 0, 255);
    int mappedY = map(currentData.coordY, 127, 255, 0, 255);

    int turnValue = constrain(mappedX - mappedY, 0, 255);

    ChangeMotorSpeed(FL, mappedX, FORWARD);
    ChangeMotorSpeed(RL, mappedX, FORWARD);

    ChangeMotorSpeed(FR, turnValue, FORWARD);
    ChangeMotorSpeed(RR, turnValue, FORWARD);
  }

  else if (currentData.coordY <= 124) {
    int mappedX = map(currentData.coordX, 128, 255, 0, 255);
    int mappedY = map(currentData.coordY, 0, 127, 0, 255);

    int turnValue = constrain(mappedX - mappedY, 0, 255);

    ChangeMotorSpeed(FL, turnValue, FORWARD);
    ChangeMotorSpeed(RL, turnValue, FORWARD);

    ChangeMotorSpeed(FR, mappedX, FORWARD);
    ChangeMotorSpeed(RR, mappedX, FORWARD);
  }
}

void loop() {
  if (radio.available() > 0) {
    radio.read(&currentData, sizeof(currentData));
    Serial.print(currentData.coordX);
    Serial.print("\t");
    Serial.print(currentData.coordY);
    Serial.print("\t");
    Serial.println(currentData.isButtonPressed);
  }

  ControlLogic(currentData);
}
