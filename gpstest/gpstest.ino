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
/** Using the Adafruit GPS library found here: https://github.com/adafruit/Adafruit_GPS **/
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h> //used by GPS
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

/** GPS **/
SoftwareSerial mySerial(8, 7);  //digital pins used with shield
Adafruit_GPS GPS(&mySerial);//GPS object
#define GPSECHO true //for debugging
//default settings
boolean usingInterrupt = false;
void useInterrupt(boolean);

/************* Setup *************/
void setup() {
    //turn on Serial monitor
    Serial.begin(115200);   //special speed for GPS
    Serial.println("Testing GPS...\n");

    //start lcd
    lcd.begin(20,4); //init display
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("GPS Test");  

    //start and initialize GPS
    GPS.begin(9600);    //default NMEA speed
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //turn on data
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); //1Hz update rate
    GPS.sendCommand(PGCMD_NOANTENNA);   //we don't have antenna
    useInterrupt(true); //constantly pull data from GPS
    delay(1000);
    
    //wait to acquire GPS signal  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Waiting for GPS");
    unsigned long startTime = millis();
    while(!GPS.fix) //wait until we get a fix on our position
    {
        lcd.setCursor(0,1);
        lcd.print("Wait Time: ");
        lcd.print((int)(millis() - startTime) / 1000); //time elapsed
        if(GPS.newNMEAreceived())
            GPS.parse(GPS.lastNMEA());
    }
    
    //initiate countdown
    lcd.clear();
    lcd.print("GPS Acquired");
    lcd.setCursor(0,1);
    lcd.print("Starting in..."); 
    lcd.setCursor(0,2);
    for(int i=10; i>0; i--)
    {
        lcd.print(i);
        lcd.print(" ");
        if(GPS.newNMEAreceived())
            GPS.parse(GPS.lastNMEA());
        delay(500);
    }
}

/************* Functions *************/
/** GPS Boilerplate Functions **/
//Interrupt is called once a millisecond, looks for anhy new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect)
{
    GPS.read();
}
//turn interrupt on and off
void useInterrupt(boolean v)
{
    if(v) {
        OCR0A = 0xAF;
        TIMSK0 |= _BV(OCIE0A);
        usingInterrupt = true;
    }else {
        TIMSK0 &= ~_BV(OCIE0A);
        usingInterrupt = false;
    }
}
/************* Loop *************/
void loop() {

}

