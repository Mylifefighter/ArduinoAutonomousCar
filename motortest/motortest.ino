//RC Code v1
//
//
//ARDUINO (UNO) SETUP:
//=====================
//Ping sensor (HC-SR04) = 5V, GND, D11 (for both trigger & echo)
//LCD Display = I2C : 5v, GND, SCL (A5) & SDA (A4)
//Adafruit GPS Shield = D7 & D8 (Uses shield, so pins used internally)
//IR Reciever (TSOP38238) = TBD
//Adafruit LSM303 Compass = TBD
//Adafruit MotorShield v2 = M1 (Accelerate), M3 (Turn)


/************* Libraries *************/ 
#include <Wire.h> //used by: motor shield
#include <Adafruit_MotorShield.h> //used by: motor shield
#include "utility/Adafruit_MS_PWMServoDriver.h" //used by: motor shield for DC motors


/************* Globals *************/

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
    AFMS.begin(); //start the motor shield

    //set the default motor speed
    driveMotor->setSpeed(NORMAL_SPEED);
    turnMotor->setSpeed(255); //full turn

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
    turnMotor->setSpeed(255);
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
}


