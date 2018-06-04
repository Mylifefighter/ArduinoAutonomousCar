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
//Adafruit LSM303 Compass = TBD
//Adafruit MotorShield v2 = M1 (Accelerate), M3 (Turn)


/************* Libraries *************/ 
#include <Wire.h> //Allows communication with I2C devices, which our LCD is
/** Using the IRLib2 library found here: https://github.com/cyborg5/IRLib2 **/
#include "IRLibAll.h"
/** Using the New LiquidCrystal 1.3.5 library found here: https://bitbucket.org/fmalpartida/ **/
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MotorShield.h> //used by: motor shield
#include "utility/Adafruit_MS_PWMServoDriver.h" //used by: motor shield for DC motors

/************* Globals *************/

/** IR **/
/****** NOTE ******/
//If using the Adafruit GPS Shield, make sure the switch next to the digital I/O is set to 'direct'
IRrecvPCI myReceiver(2); //create a reciever object to listen on pin 2
IRdecodeNEC myDecoder; //create a decoder object that uses the NEC protocol of our miniremote
uint32_t Previous;//handles NEC repeat codes
#define MY_PROTOCOL NEC
#define STOP 0xfd609f

/** LCD **/
/**Note: Most displays use I2C address 0x27 but a few use 0x3F**/
//set the LCD address to 0x27 for 20 chars 4 line display
/**
 **
 **                   addr, en,rw,rs,d4,d5,d6,d7,bl,blpol  **/
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

/** Motors **/
//Create the motor shield objects
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

//Create our motors
Adafruit_DCMotor *driveMotor = AFMS.getMotor(1); //Using port M1
Adafruit_DCMotor *turnMotor = AFMS.getMotor(3); //Using port M3

//Motor speeds range: 0-255
#define FAST_SPEED 150
#define NORMAL_SPEED 125
#define TURN_SPEED 100
#define SLOW_SPEED 75

/************* Setup *************/
void setup() {
    myReceiver.enableIRIn(); //start the receiver

    lcd.begin(20,4); //init display
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
    lcd.backlight();
    AFMS.begin(); //start the motor shield
    //set the default motor speed
    driveMotor->setSpeed(NORMAL_SPEED);
    turnMotor->setSpeed(255); //full turn

    
    lcd.print("Starting in...");
    lcd.setCursor(0,2);
    for(int i=10; i>0; i--)
    {
        lcd.print(i);
        lcd.print(" ");
        delay(500);
    }
    //run the motors
    /*
    ** Motors can run in 3 directions:
    ** FORWARD: motor spins forward
    ** BACKWARD: motor spins backward
    ** RELEASE: cut power to motor, stops rotation, does not apply braking
    ** If FORWARD or BACKWARD do match the actual direction you vehicle
    ** goes, just switch the motor leads on the shield
    */
}

/************* Loop *************/
void loop() {

    /** Write chars to LCD **/
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Push to start"); 
    myReceiver.enableIRIn();
      
    if(myReceiver.getResults()) {
        myDecoder.decode();
        if(myDecoder.protocolNum==MY_PROTOCOL) {
            if(myDecoder.value==0xFFFFFFFF)
                myDecoder.value=Previous;
            if(myDecoder.value==STOP) {
                //Should only get here if STOP button was pressed
                lcd.clear();
                lcd.setCursor(3,0);
                lcd.print("Starting"); 
                delay(5000);
                lcd.clear();
                uint8_t i;

                //Make the drive motor go forward to top speed and then back down to zero
                driveMotor->setSpeed(TURN_SPEED);
                driveMotor->run(FORWARD);
                delay(10000);
                //Slow down the motor that way we don't kill it
                driveMotor->setSpeed(200);
                delay(10);
                driveMotor->setSpeed(100);
                delay(10);
                driveMotor->setSpeed(50);
                driveMotor->setSpeed(0);
                    
                driveMotor->run(RELEASE);

                //Make the drive motor go backward to top speed and then back down to zero
                driveMotor->setSpeed(TURN_SPEED);
                driveMotor->run(BACKWARD);
                delay(10000);
                driveMotor->setSpeed(200);
                delay(10);
                driveMotor->setSpeed(100);
                delay(10);
                driveMotor->setSpeed(50);
                driveMotor->setSpeed(0);

                //Completely stop the drive motor
                driveMotor->run(RELEASE);

                //Turn the wheels
                turnMotor->setSpeed(TURN_SPEED);
                turnMotor->run(FORWARD); //This should be a right hand turn
                delay(1000);
                turnMotor->setSpeed(200);
                delay(5);
                turnMotor->setSpeed(100);
                delay(5);
                turnMotor->setSpeed(50);
                turnMotor->setSpeed(0);

                turnMotor->run(RELEASE);
                delay(1000);
                }else
                {
                    lcd.clear();
                    lcd.setCursor(3,0); //start at char 4 on line 0
                    lcd.print("STOP was not pressed"); 
                    delay(5000);
                }
            }

            Previous=myDecoder.value;
            lcd.clear();
            lcd.setCursor(3,0); //start at char 4 on line 0
            lcd.print("Done"); 
            delay(5000);
            myReceiver.enableIRIn();
        }


}



