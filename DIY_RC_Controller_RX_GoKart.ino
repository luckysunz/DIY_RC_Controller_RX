/*
  OPENRC GoKart
  DIY RC Receiver - Servos and ESC motor control
  last change 01/2025
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <elapsedMillis.h>

#define CE_PIN 3
#define CSN_PIN 2

const byte thisSlaveAddress[5] = { 'M', 'A', 'R', '2', '3' };
//{ 'R', 'x', 'A', 'A', 'A' };
RF24 radio(CE_PIN, CSN_PIN);
elapsedMillis printTime;
elapsedMillis recieveTime;
elapsedMillis checkTime;

volatile bool newData = false;
// create servo object to control the ESC
Servo servo2;  //servo2 = CH2
Servo servo3;
Servo esc1;    //ESC CH9 BEC 
Servo esc2;    //ESC CH5
// Servo potservo5;
// Servo potservo6;

int servo2Value;
int servo3Value;
int btn1Value;
int btn2Value;
int esc1Value;
int esc2Value;
int tSwitch1Value;
int tSwitch2Value;
int poti1Value;
int poti2Value;


// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button;
  byte j2PotX;
  byte j2PotY;
  byte j2Button;
  byte poti1;
  byte poti2;
  byte tSwitch1;
  byte tSwitch2;
  byte button1;
  byte button2;
  byte button3;
  byte button4;
};
Data_Package rdata;  //Create a variable with the above structure

void setup() {

  Serial.begin(115200);
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  delay(400);
  radio.setAutoAck(true);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, and RF24_PA_max.
  radio.openReadingPipe(1, thisSlaveAddress);
  delay(200);
  //radio.openReadingPipe(0, thisSlaveAddress);
  radio.startListening();
  delay(300);
  Serial.print("RX Listening on Channel: ");
  Serial.println(radio.getChannel());
  resetData();
  
  //RX mit 4 Kanälen von CH2 - CH5
  servo2.attach(5);  // D5 - CH2
  servo3.attach(6);  //D6 -  CH3
  esc1.attach(10, 900, 2050); // D10 - CH9 w. BEC
  //esc2.attach(8, 1000, 1500); //  D8  - CH5
  delay(200);

  //initial esc -> calibrate
  servo2.write(90);
  delay(10);
  servo3.write(90);
delay(300);
  esc1.writeMicroseconds(1500);
  delay(300);
  
  esc1.writeMicroseconds(1000);
  delay(10);
  esc2.writeMicroseconds(2000);
  delay(400);
  esc1.writeMicroseconds(2000);
  delay(10);
  esc2.writeMicroseconds(1000);
  delay(400);
  esc1.write(90);
  //esc1.writeMicroseconds(1500);
}
void loop() {

  if (recieveTime >= 200) {
    recieveTime = 0;

    if (radio.available()) {
      radio.read(&rdata, sizeof(Data_Package));  //Daten vom Sender werden in rdata eingelesen  
      //rdata.poti1 = analogSmooth(rdata.poti1);
    } else {
      //Serial.print(":::::::");
      //while (1) {}  // hold in infinite loop TEST
    }
  }

  if (printTime >= 1000) {
    printTime = 0;
    Serial.print("J1X: ");
    Serial.print(rdata.j1PotX);
    Serial.print(" J1Y: ");
    Serial.print(rdata.j1PotY);
    Serial.print(" J2X: ");
    Serial.print(rdata.j2PotX);
    Serial.print(" J2Y: ");
    Serial.print(rdata.j2PotY);
    Serial.print(" Poti1: ");
    Serial.print(rdata.poti1);
    Serial.print(" Poti2: ");
    Serial.print(rdata.poti2);
    Serial.print(" TSwitch1: ");
    Serial.print(rdata.tSwitch1);
    Serial.print(" TSwitch2: ");
    Serial.println(rdata.tSwitch2);
    
  }

//Reset mit Button4
  if (rdata.button4 == 0) {
    resetData();
    Serial.println("RESET");
    delay(400);    
    rdata.button4 = 1;
  }
//Begrenzung der Servos mit Toggle Switch2
  if (rdata.tSwitch2 == 1) {
    //servo1Value = map(rdata.j1PotY, 0, 255, 115, 65);  // Map the receiving value form 0 to 255 to 0 to 180(degrees), values used for controlling servos
    servo2Value = map(rdata.j2PotX, 0, 255, 30, 150);
    servo3Value = map(rdata.poti2, 0, 255, 40, 140);
    esc1Value = map(rdata.j1PotY, 0, 330, 1000, 2000);
    esc2Value = map(rdata.j1PotX, 0, 255, 2000, 800);

  } else {
    servo2Value = map(rdata.j2PotX, 0, 255, 0, 180);
    servo3Value = map(rdata.poti2, 0, 255, 20, 160);
    esc1Value = map(rdata.j1PotY, 0, 255, 1000, 2000);
    esc2Value = map(rdata.j1PotX, 0, 255, 2500, 800);
  }
  
  //Controlling servos
  servo2.write(servo2Value);
  //delay(1);
  //servo3.write(servo3Value);

  esc1.writeMicroseconds(esc1Value);  // Send the PWM control singal to the ESC
  delay(5);
  esc2.writeMicroseconds(esc2Value);  // Send the PWM control singal to the ESC
  delay(2);
  
  //servo4.write(btn1Value);
  // escValue = map(rdata.button1, 0, 255);  // Map the receiving value form 127 to 255 to  1000 to 2000, values used for controlling ESCs
}
void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  rdata.j1PotX = 127;
  rdata.j1PotY = 127;
  rdata.j2PotX = 127;
  rdata.j2PotY = 127;
  rdata.button1 = 1;
  rdata.button2 = 1;
  rdata.button3 = 1;
  rdata.button4 = 1;
  servo2.write(90);
  servo3.write(90);
  esc1.writeMicroseconds(1000);
  esc2.writeMicroseconds(1000);
}