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

/************* Globals *************/
IRrecvPCI myReceiver(2); //create a reciever object to listen on pin 2

IRdecodeNEC myDecoder; //create a decoder object that uses the NEC protocol of our miniremote

/****** NOTE ******/
//If using the Adafruit GPS Shield, make sure the switch next to the digital I/O is set to 'direct'

/************* Setup *************/
void setup() {
    Serial.begin(9600); //this will allow us to view our IR signals 
    delay(2000);
    myReceiver.enableIRIn(); //start the receiver
    Serial.println(F("ready to receive IR signals:"));
}
/************* Loop *************/
void loop() {
    //loop until we get a complete signal
    if(myReceiver.getResults()) {
        myDecoder.decode();
        myDecoder.dumpResults(true);
        /** specific value codes can be accessed with myDecoder.value **/
        myReceiver.enableIRIn(); //restart receiver
    }
}

