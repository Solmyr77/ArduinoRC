#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define FORWARD 0
#define REVERSE 1

#define LEFT 0
#define RIGHT 1

#define LEFT_MOTORS 0
#define RIGHT_MOTORS 1

#define FL_MOTOR_PWM_PIN 6
#define FR_MOTOR_PWM_PIN 5
#define RL_MOTOR_PWM_PIN 3
#define RR_MOTOR_PWM_PIN 9

#define FL_MOTOR_DIR_PIN 7
#define FR_MOTOR_DIR_PIN 4
#define RL_MOTOR_DIR_PIN 2
#define RR_MOTOR_DIR_PIN 8

bool ledState = false;

RF24 radio(A1, 10); // CE, CSN

const byte address[6] = "00001";

struct Data {
  byte coordX = 124;
  byte coordY = 127;
  bool isButtonPressed = false;
};

Data currentData;

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A2, OUTPUT);
  Serial.begin(115200);
  radio.begin();
  //Append ACK packet from the receiving radio back to the transmitting radio
  radio.setAutoAck(true); //(true|false)
  //Set the transmission datarate
  radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
  //Greater level = more consumption = longer distance
  radio.setPALevel(RF24_PA_MIN); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX)
  //Default value is the maximum 32 bytes1
  radio.setPayloadSize(sizeof(currentData));
  //Act as receiver
  radio.openReadingPipe(0, address);
  radio.startListening();
}

void setMotorSpeed(int motorSelection, int speedValue, int motorDirection) {

  if (motorDirection == REVERSE) {
    speedValue = map(speedValue, 0, 255, 255, 0);
  }

  // Left motor logic
  if (motorSelection == LEFT_MOTORS) {
    digitalWrite(FL_MOTOR_DIR_PIN, motorDirection);
    analogWrite(FL_MOTOR_PWM_PIN, speedValue);

    digitalWrite(RL_MOTOR_DIR_PIN, motorDirection);
    analogWrite(RL_MOTOR_PWM_PIN, speedValue);

    /*Serial.print("\tB: ");
    Serial.print(speedValue);

    Serial.print("\tBR: ");
    Serial.print(motorDirection);*/
  }

  // Right motor logic
  else {
    digitalWrite(FR_MOTOR_DIR_PIN, motorDirection);
    analogWrite(FR_MOTOR_PWM_PIN, speedValue);

    digitalWrite(RR_MOTOR_DIR_PIN, motorDirection);
    analogWrite(RR_MOTOR_PWM_PIN, speedValue);

    /*Serial.print("\tJ: ");
    Serial.println(speedValue);

    Serial.print("\tJR: ");
    Serial.print(motorDirection);*/
  }
}

void TurnMotorsOff() {
  setMotorSpeed(LEFT_MOTORS, 0, 0);
  setMotorSpeed(RIGHT_MOTORS, 0, 0);
}

void TankTurn(int joystickYValue) {
  /*Serial.print(joystickYValue);
    Serial.println("Tankturn");*/

  const int speedMultiplier = 1;

  int normalizedY = currentData.coordY - 127;
  int turnDirection = normalizedY >= 0 ? RIGHT : LEFT;

  int motorSpeedValue = map(abs(normalizedY), 0, 127, 0, 255);
  int motorSpeedValueInverse = map(motorSpeedValue, 0, 255, 255, 0);

  motorSpeedValue = constrain(motorSpeedValue, 0, 255);

  if (turnDirection == LEFT) {
    setMotorSpeed(RIGHT_MOTORS, motorSpeedValue, FORWARD);
    setMotorSpeed(LEFT_MOTORS, motorSpeedValue, REVERSE);
  }
  else {
    setMotorSpeed(RIGHT_MOTORS, motorSpeedValue, REVERSE);
    setMotorSpeed(LEFT_MOTORS, motorSpeedValue, FORWARD);
  }
}

void ControlLogic(Data currentData) {
  if (currentData.coordX > 120 && currentData.coordX < 128) {
    if (currentData.coordY > 123 && currentData.coordY < 131) {
      TurnMotorsOff();
    }
    else {
      TankTurn(currentData.coordY);
    }
  }
  else {
    int normalizedX = currentData.coordX - 127;

    int rotationDirection = normalizedX >= 0 ? FORWARD : REVERSE;

    int turnValue = map(currentData.coordY, 0, 255, -255, 255);

    int motorSpeed = map(abs(normalizedX), 0, 127, 0, 255);

    int leftMotorSpeed = motorSpeed;
    int rightMotorSpeed = motorSpeed;

    if (rotationDirection == FORWARD && (currentData.coordY > 123 && currentData.coordY < 131)) {
      leftMotorSpeed = motorSpeed - turnValue;
      rightMotorSpeed = motorSpeed + turnValue;
    }
    else {
      leftMotorSpeed = motorSpeed + turnValue;
      rightMotorSpeed = motorSpeed - turnValue;
    }

    leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);
    rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);

    setMotorSpeed(LEFT_MOTORS, leftMotorSpeed, rotationDirection);
    setMotorSpeed(RIGHT_MOTORS, rightMotorSpeed, rotationDirection);
  }
}

void ControlLED(bool isOn) {
  if (isOn) {
    digitalWrite(A2, HIGH);
  }
  else {
    digitalWrite(A2, LOW);
  }
}

void loop() {
  radio.startListening();

  if (radio.available() > 0) {
    radio.read(&currentData, sizeof(currentData));
    /*Serial.print(currentData.coordX);
      Serial.print("\t");
      Serial.print(currentData.coordY);
      Serial.print("\t");
      Serial.println(currentData.isButtonPressed);*/
  }

  ControlLED(currentData.isButtonPressed);
  ControlLogic(currentData);
}
