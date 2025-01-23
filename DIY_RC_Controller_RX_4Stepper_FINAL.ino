/*
    DIY Arduino based RC Transmitter Project
              == Receiver Code ==
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
// Include the AccelStepper Library
#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//#define ENAB_PIN 13  //EN ==LOW um zu aktivieren
#define CE_PIN 9
#define CSN_PIN 10

const byte thisSlaveAddress[5] = { 'F', 'M', '1', '4', 'c' };
//{ 'R', 'x', 'A', 'A', 'A' };
RF24 radio(CE_PIN, CSN_PIN);

volatile bool newData = false;
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button;
  byte pot1;
  byte tSwitch1;
};
Data_Package rdata;  //Create a variable with the above structure
Data_Package cdata;  //Datebpackete zum Vergleich

//============Analog smoothing
const int numReadings = 4;
int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int average = 0;


//============================

// Ausgang Stepper Motor angechlossen

bool XisActive = false;
bool YisActive = false;
bool ZisActive = false;
bool CMDActive = false;

// Stepper Motor X
const int XstepPin = 2;  //X.STEP
const int XdirPin = 5;   // X.DIR
const int Xstepmode = 4;
// Stepper Motor Y
const int YstepPin = 3;  //Y.STEP
const int YdirPin = 6;   // Y.DIR
const int Ystepmode = 4;
// Stepper Motor Z
const int ZstepPin = 4;  //Z.STEP
const int ZdirPin = 7;   // Z.DIR
const int Zstepmode = 2;

//AccelStepper myStepper(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper XStepper(AccelStepper::DRIVER, XstepPin, XdirPin);
AccelStepper YStepper(AccelStepper::DRIVER, YstepPin, YdirPin);
AccelStepper ZStepper(AccelStepper::DRIVER, ZstepPin, ZdirPin);

//bool EnabStepper = false;
//float mspeed;
//int state;

float xpos = 0;
float ypos = 0;
float zpos = 0;  //Poti Mittelstellung
elapsedMillis printTime;
elapsedMillis recieveTime;
elapsedMillis checkTime;


void setup() {
  Serial.begin(115200);
  //Stepper BOARD ON SLEEP PIN von D13
  //pinMode(ENAB_PIN, OUTPUT);
  // pinMode(XstepPin, OUTPUT);
  // pinMode(XdirPin, OUTPUT);
  // pinMode(YstepPin, OUTPUT);
  // pinMode(YdirPin, OUTPUT);


  //num Readings mit Startwerten füllen
  for (int i = 0; i == numReadings; i++) {
    readings[i] = 130;
  }

  //radio.begin();
  //digitalWrite(ENAB_PIN, LOW);
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }
  //digitalWrite(ENAB_PIN, LOW);
  //Serial.println(digitalRead(ENAB_PIN));
  delay(400);
  radio.setAutoAck(true);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, and RF24_PA_max.
  radio.openReadingPipe(1, thisSlaveAddress);
  delay(200);
  radio.openReadingPipe(0, thisSlaveAddress);
  radio.startListening();
  Serial.println("RX Listening");

  XStepper.setMaxSpeed(30.0);
  YStepper.setMaxSpeed(30.0);
  ZStepper.setMaxSpeed(30.0);

  delay(300);
  ZStepper.setCurrentPosition(0);
  YStepper.setCurrentPosition(0);
  XStepper.setCurrentPosition(0);
}

//=============

void loop() {

  getData();
  checkData();
  if (CMDActive) {
    myStepper();
  }
  //showData();
  showStepperPos();
  if(newData){
  resetStepperPos();
  }
}

//==============

void getData() {
  if (recieveTime >= 200) {
    recieveTime = 0;

    if (radio.available()) {
      radio.read(&rdata, sizeof(Data_Package));  //Daten vom Sender werden in rdata eingelesen
      rdata.pot1 = analogSmooth(rdata.pot1);
    }
    if ((cdata.j1PotX == rdata.j1PotX) && (cdata.j1PotY == rdata.j1PotY) && (cdata.j1Button == rdata.j1Button) && (cdata.pot1 == rdata.pot1)) {
      newData = false;
      ZisActive = false;
      //CMDActive = false;

      
    } else {
      newData = true;
      ZisActive = true;
      
    }
  } else {
    //newData = false;
  }
}

void checkData() {
  if (checkTime >= 401) {
    checkTime = 0;
    cdata.j1PotX = rdata.j1PotX;
    cdata.j1PotY = rdata.j1PotY;
    cdata.pot1 = rdata.pot1;
    cdata.j1Button = rdata.j1Button;
  }
}
void resetStepperPos() {
  if (rdata.j1Button == 0) {
    YStepper.setCurrentPosition(0);
    XStepper.setCurrentPosition(0);
    ZStepper.setCurrentPosition(analogSmooth(rdata.pot1));
    if (!CMDActive)
    {
        CMDActive = true;
      }
    else 
    { CMDActive = false;
    }
  }
}

void showStepperPos() {
  if (printTime >= 1000) {
    printTime = 0;
    Serial.print(" XPos:_");
    Serial.print(XStepper.currentPosition());
    Serial.print(" YPos:_");
    Serial.print(YStepper.currentPosition());
    Serial.print(" ZPos:_");
    Serial.print(ZStepper.currentPosition());
    Serial.print(" rdata.pot1: ");
    Serial.print(rdata.pot1);
    Serial.print(" NewData: ");
    Serial.print(newData);
    Serial.print(" CMDActive: ");
    Serial.println(CMDActive);
  }
}
void showData() {
  if ((newData == true) && (printTime >= 900)) {
    Serial.print("Data received ");
    Serial.print("j1PotX: ");
    Serial.print(rdata.j1PotX);
    Serial.print("; j1PotY: ");
    Serial.print(rdata.j1PotY);
    Serial.print("; j1button: ");
    Serial.print(rdata.j1Button);
    Serial.print("; Poti: ");
    Serial.println(rdata.pot1);
  }
}

void zAnalogturn() {
  if (ZisActive) {
    XisActive = false;
    YisActive = false;

    ZStepper.moveTo(cdata.pot1);
    ZStepper.setSpeed(35);
    ZStepper.runSpeedToPosition();
    delayMicroseconds(5);
  }
}

void xDigitelturn() {
  if (XisActive) {
    XStepper.moveTo((XStepper.currentPosition() + 1));
    XStepper.setSpeed(17);
    delayMicroseconds(5);
    XStepper.runSpeedToPosition();
    if (XStepper.distanceToGo() == 0) {
      YisActive = true;
      ZisActive = true;
    }
  }
}
void xDigitelturnRev() {
  if (XisActive) {
    XStepper.moveTo((XStepper.currentPosition() - 1));
    XStepper.setSpeed(17);
    XStepper.runSpeedToPosition();
    delayMicroseconds(5);
    if (XStepper.distanceToGo() == 0) {
      YisActive = true;
      ZisActive = true;
    }
  }
}

void yDigitelturnDown() {
  if (YisActive) {
    YStepper.moveTo(YStepper.currentPosition() + 1);
    YStepper.setSpeed(20);
    YStepper.runSpeedToPosition();
    delayMicroseconds(10);
    if (YStepper.distanceToGo() == 0) {
      XisActive = true;
      ZisActive = true;
    }
  }
}
void yDigitelturnUp() {
  if (YisActive) {
    YStepper.moveTo((YStepper.currentPosition() - 1));
    YStepper.setSpeed(20);
    YStepper.runSpeedToPosition();
    delayMicroseconds(10);

    if (YStepper.distanceToGo() == 0) {
      XisActive = true;
      ZisActive = true;
    }
  }
}

void myStepper() {
  //XSTEPPER Control
  if ((rdata.j1PotX > 125) && (rdata.j1PotX < 131)) {
    XisActive = false;
  }
  if (rdata.j1PotX > 130) {  //RECHTS drehen
    XisActive = true;
    ZisActive = false;
    YisActive = false;
    xDigitelturnRev();
  }
  if (rdata.j1PotX < 120) {  //Links drehen
    XisActive = true;
    ZisActive = false;
    YisActive = false;
    xDigitelturn();
  }

  //YSTEPPER Control
  if ((rdata.j1PotY > 125) && (rdata.j1PotX < 131)) {
    YisActive = false;
  }
  if (rdata.j1PotY > 130) {  //HOCH drehen
    YisActive = true;
    ZisActive = false;
    XisActive = false;
    yDigitelturnUp();
  }
  if (rdata.j1PotY < 125) {  //RUNTER drehen
    YisActive = true;
    ZisActive = false;
    XisActive = false;
    yDigitelturnDown();
  }

  ///ZStepper Control am Poti
  zAnalogturn();
}


byte analogSmooth(byte myPoti) {
  // subtract the last reading:
  total = total - readings[readIndex];
  readings[readIndex] = myPoti;  //analogRead(inputPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  average = total / numReadings;
  //Serial.println(average);
  delay(4);
  return average;
}
