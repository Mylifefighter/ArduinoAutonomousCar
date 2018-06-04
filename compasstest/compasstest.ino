//RC Code v1
//
//
//ARDUINO (UNO) SETUP:
//=====================
//Ping sensor (HC-SR04) = 5V, GND, D11 (for both trigger & echo)
//LCD Display (LCM1602 IIC V1) = I2C : 5v, GND, SCL (A5) & SDA (A4)
//Adafruit Ultimate GPS Logger Shield = D7 & D8 (Uses shield, so pins used internally)
//IR Receiver (TSOP38238) = 5v, GND, D2
//Adafruit Mini Remote Control for use with IR Receiver
//Adafruit LSM303 Compass = VIN, GND, SDA (A4), SCL (A5)
//Adafruit MotorShield v2 = M1 (Accelerate), M3 (Turn)

/************* Libraries *************/
#include <Wire.h>
#include <math.h>//for M_PI
/** Using the Adafruit Sensor library found here: https://github.com/adafruit/Adafruit_Sensor **/
#include <Adafruit_Sensor.h>
/** Using the Adafruit LSM303 library found here: https://github.com/adafruit/Adafruit_LSM303DLHC **/
#include <Adafruit_LSM303_U.h>
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

/** Compass **/
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12334); //Create compass object with a unique id

/************* Setup *************/
void setup () {
    lcd.begin(20,4); //init display
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Compass Test");  
    
    //Attempt to initialize compass
    if(!mag.begin())
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Could not detect compass...check wiring");
        while(1);
    }
}

/************* Loop *************/
void loop() {
    //create compass event
    sensors_event_t event;
    mag.getEvent(&event); 
    
    float Pi = M_PI;
    
    //Calulating angle of vector y,x
    float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / Pi;

    //Normalize to 0-360
    if (heading < 0)
        heading = 360 + heading;
        
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Compass Heading: ");
    lcd.setCursor(3,2);
    lcd.print(heading);
    delay(500);
}

