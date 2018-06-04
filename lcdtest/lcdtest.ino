//
//
//ARDUINO (UNO) SETUP:
//=====================
//Ping sensor (HC-SR04) = 5V, GND, D11 (for both trigger & echo)
//LCD Display (LCM1602 IIC V1) = I2C : 5v, GND, SCL (A5) & SDA (A4)
//Adafruit GPS Shield = D7 & D8 (Uses shield, so pins used internally)
//IR Reciever (TSOP38238) = TBD
//Adafruit LSM303 Compass = TBD
//Adafruit MotorShield v2 = M1 (Accelerate), M3 (Turn)


/************* Libraries *************/ 
/** Using the New LiquidCrystal 1.3.5 library found here: https://bitbucket.org/fmalpartida/ **/
#include <LiquidCrystal_I2C.h>
#include <Wire.h> //Allows communication with I2C devices, which our LCD is

/************* Globals *************/
/**Note: Most displays use I2C address 0x27 but a few use 0x3F**/
//set the LCD address to 0x27 for 20 chars 4 line display
/**
 **
 **                   addr, en,rw,rs,d4,d5,d6,d7,bl,blpol  **/
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


/************* Setup *************/
void setup() {
  lcd.begin(20,4); //init display

  /** 3 blinks of backlight **/
   for(int i=0; i<3; i++)
   {
      lcd.backlight();
      delay(250);
      lcd.noBacklight();
      delay(250);
   }
   lcd.backlight();

   /** Write chars to LCD **/
   lcd.setCursor(3,0); //start at char 4 on line 0
   lcd.print("This is working!");
   delay(1000);
   lcd.setCursor(3,1);
   lcd.print("line # 2");
   delay(1000);
   lcd.setCursor(0,2);
   lcd.print("line # 3");
   delay(1000);
   lcd.setCursor(0,3);
   lcd.print("line # 4");
   
}
/************* Loop *************/
void loop() {
  
}

