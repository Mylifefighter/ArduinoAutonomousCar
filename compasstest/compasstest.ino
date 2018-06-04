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

/************* Globals *************/
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12334); //Create compass object with a unique id

/************* Setup *************/
void setup () {
    Serial.begin(9600);
    Serial.println("Compass Test\n");  
    
    //Attempt to initialize compass
    if(!mag.begin())
    {
        Serial.println("Could not detect compass...check wiring");
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
    
    Serial.print("Compass Heading: ");
    Serial.println(heading);
    delay(500);
}

