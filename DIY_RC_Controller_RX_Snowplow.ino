/*
  DIY RC Receiver - Servos and Brushless motors control
  by Dejan, www.HowToMechatronics.com
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
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
//Servo esc;     // create servo object to control the ESC
Servo servo1;  //servo1 = CH1
Servo servo2;  //servo2 = CH2
Servo servo3;
Servo servo4;
Servo esc6;
Servo esc5;
// Servo potservo5;
// Servo potservo6;

int servo1Value;
int servo2Value;
int servo3Value;
int servo4Value;
int btn1Value;
int btn2Value;
int esc5Value;
int esc6Value;
int tSwitch1Value;
int tSwitch2Value;

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
  //esc.attach(10);    // Arduino digital pin D10 - CH9 on PCB board
  servo1.attach(4);  // D4 - CH1
  servo2.attach(5);  // D5 - CH2
  servo3.attach(6);  //D6 -  CH3
  servo4.attach(7);  //D7 -  CH4
  esc5.attach(8, 1000, 1500);
  esc6.attach(9, 1000, 1500);
  delay(100);

  //initial esc
  servo1.write(0);
  servo2.write(0);
  esc5.writeMicroseconds(500);
  esc6.writeMicroseconds(500);
  delay(300);
  servo1.write(180);
  servo2.write(180);
  esc5.writeMicroseconds(2500);
  esc6.writeMicroseconds(2500);
  delay(200);
}
void loop() {

  if (recieveTime >= 200) {
    recieveTime = 0;

    if (radio.available()) {
      radio.read(&rdata, sizeof(Data_Package));  //Daten vom Sender werden in rdata eingelesen
      servo4.write(30);
      //rdata.pot1 = analogSmooth(rdata.pot1);
    } else {

      servo4.write(0);
    }
  }

  if (printTime >= 1000) {
    printTime = 0;
    Serial.print("J1X: ");
    Serial.print(rdata.j1PotX);
    Serial.print(" J1Y: ");
    Serial.print(rdata.j1PotY);
    Serial.print("J2X: ");
    Serial.print(rdata.j2PotX);
    Serial.print(" J2Y: ");
    Serial.print(rdata.j2PotY);
    Serial.print(" Poti1: ");
    Serial.print(rdata.poti1);
    Serial.print(" Poti2: ");
    Serial.print(rdata.poti2);
    Serial.print(" TSwitch2: ");
    Serial.print(rdata.tSwitch2);
    Serial.print(" J1BTN: ");
    Serial.print(rdata.j1Button);
    Serial.print(" J2BTN: ");
    Serial.print(rdata.j2Button);
    Serial.print(" BTN1: ");
    Serial.print(rdata.button1);
    Serial.print(" BTN2: ");
    Serial.print(rdata.button2);
    Serial.print(" BTN3: ");
    Serial.print(rdata.button3);
    Serial.print(" BTN4: ");
    Serial.println(rdata.button4);
  }


  if (rdata.button2 == 0) {
    resetData();
    rdata.button2 = 1;
  }
//Begrenzung der Servos mit Toggle Switch2
  if (rdata.tSwitch2 == 1) {
    servo1Value = map(rdata.j1PotY, 0, 255, 115, 65);  // Map the receiving value form 0 to 255 to 0 to 180(degrees), values used for controlling servos
    servo2Value = map(rdata.j2PotY, 0, 255, 115, 65);
    servo3Value = map(rdata.poti1, 0, 255, 0, 180);
    esc5Value = map(rdata.j1PotX, 0, 255, 800, 2000);
    esc6Value = map(rdata.j1PotX, 0, 255, 800, 2000);


  } else {
    servo1Value = map(rdata.j1PotY, 0, 255, 150, 30);  // Map the receiving value form 0 to 255 to 0 to 180(degrees), values used for controlling servos
    servo2Value = map(rdata.j2PotY, 0, 255, 150, 30);
    servo3Value = map(rdata.poti1, 0, 255, 0, 180);
    
    esc5Value = map(rdata.j1PotX, 0, 255, 500, 2500);
    esc6Value = map(rdata.j1PotX, 0, 255, 500, 2500);
  }
  //Controlling servos


  servo1.write(servo1Value);
  delay(5);
  servo2.write(servo2Value);
  delay(5);
  servo3.write(servo3Value);

  esc5.writeMicroseconds(esc5Value);  // Send the PWM control singal to the ESC
  delay(5);
  esc6.writeMicroseconds(esc6Value);  // Send the PWM control singal to the ESC

  // btn1Value = map(rdata.button1, 0, 0, 1, 255);
  // tSwitch2Value = map(rdata.tSwitch2, 0, 0, 1, 255);
  //potservo6.write(tSwitch2Value);
  //servo4.write(btn1Value);
  // escValue = map(rdata.button1, 0, 255);  // Map the receiving value form 127 to 255 to  1000 to 2000, values used for controlling ESCs
}
void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  rdata.j1PotX = 127;
  rdata.j1PotY = 127;
  rdata.j2PotX = 127;
  rdata.j2PotY = 127;
  // rdata.j1Button = 1;
  // rdata.j2Button = 1;
  // rdata.pot1 = 0;
  // rdata.pot2 = 0;
  // data.tSwitch1 = 1;
  // data.tSwitch2 = 1;
  rdata.button1 = 1;
  rdata.button2 = 1;
  rdata.button3 = 1;
  rdata.button4 = 1;
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
  servo4.write(0);
  esc5.writeMicroseconds(1000);
  esc6.writeMicroseconds(1000);
}