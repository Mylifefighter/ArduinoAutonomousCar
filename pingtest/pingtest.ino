//RC Code v1
//
//
//ARDUINO (UNO) SETUP:
//=====================
//Ping sensor (HC-SR04) = 5V, GND, D11 (for both trigger & echo)
//LCD Display (LCM1602 IIC V1) = I2C : 5v, GND, SCL (A5) & SDA (A4)
//Adafruit Ultimate GPS Logger Shield = D7 & D8 (Uses shield, so pins used internally) Make sure the logging switch is set to 'soft. serial'
//IR Receiver (TSOP38238) = 5v, GND, D2
//Adafruit Mini Remote Control for use with IR Receiver
//Adafruit LSM303 Compass = VIN, GND, SDA (A4), SCL (A5)
//Adafruit MotorShield v2 = M1 (Accelerate), M3 (Turn)

/************* Libraries *************/
#include <Wire.h>
#include <NewPing.h> //for ping sensor
/** Using the New LiquidCrystal 1.3.5 library found here: https://bitbucket.org/fmalpartida/ **/
#include <LiquidCrystal_I2C.h>

/************* Globals *************/
/** LCD **/
/**Note: Most displays use I2C address 0x27 but a few use 0x3F**/
//set the LCD address to 0x27 for 20 chars 4 line display
/**
 **
 **                   addr, en,rw,rs,d4,d5,d6,d7,bl,blpol  **/
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

/** Ping sensor **/
#define TRIGGER_PIN 11
#define ECHO_PIN 11
#define MAX_DISTANCE_CM 250 //max dist we want to ping for
#define MAX_DISTANCE_IN (MAX_DISTANCE_CM/2.5) //max dist to ping for in inches
int sonarDistance;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE_CM);  //create ping object

/************* Setup *************/
void setup() {
  //start lcd
  lcd.begin(20,4); //init display
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Ping Test");
}
/************* Loop *************/
void loop() {
  delay(50);
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Ping: ");
  lcd.setCursor(3,2);
  lcd.print(sonar.ping_cm()); //send ping, get distance
  lcd.setCursor(7,2);
  lcd.print("cm");
}

