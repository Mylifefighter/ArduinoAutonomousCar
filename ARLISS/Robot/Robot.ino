//RC Code v1
//
//Code based on instructable found here: http://www.instructables.com/id/Arduino-Powered-Autonomous-Vehicle/
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
/** Using the Adafruit LSM303 library found here: https://github.com/adafruit/Adafruit_LSM303DLHC **/
#include <Adafruit_LSM303_U.h>
/** Using the Adafruit Sensor library found here: https://github.com/adafruit/Adafruit_Sensor **/
#include <Adafruit_Sensor.h>
#include <NewPing.h> //for ping sensor
/** Using the New LiquidCrystal 1.3.5 library found here: https://bitbucket.org/fmalpartida/ **/
#include <LiquidCrystal_I2C.h>
/** Using the Adafruit GPS library found here: https://github.com/adafruit/Adafruit_GPS **/
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h> //used by GPS
#include <Adafruit_MotorShield.h> //used by: motor shield
#include "utility/Adafruit_MS_PWMServoDriver.h" //used by: motor shield for DC motors

/************* Classes *************/
//moving average for sonar functionality
template <typename V, int N> class MovingAverage
{
public:
    /*
     * @brief Class constructor.
     * @param n the size of the moving average window.
     * @param def the default value to initialize the average.
     */
    MovingAverage(V def = 0) : sum(0), p(0)
    {
        for (int i = 0; i < N; i++) {
            samples[i] = def;
            sum += samples[i];
        }
    }
    
    /*
     * @brief Add a new sample.
     * @param new_sample the new sample to add to the moving average.
     * @return the updated average.
     */
    V add(V new_sample)
    {
        sum = sum - samples[p] + new_sample;
        samples[p++] = new_sample;
        if (p >= N)
            p = 0;
        return sum / N;
    }
    
private:
    V samples[N];
    V sum;
    V p;
};

//custom class to manage GPS waypoints
class  waypointClass
{
    
  public:
    waypointClass(float pLong = 0, float pLat = 0)
      {
        fLong = pLong;
        fLat = pLat;
      }
      
    float getLat(void) {return fLat;}
    float getLong(void) {return fLong;}

  private:
    float fLong, fLat;
      
  
};  // waypointClass

/************* Globals *************/

/** Compass **/
Adafruit_LSM303_Mag_Unified compass = Adafruit_LSM303_Mag_Unified(12334); //Create compass object with a unique id
sensors_event_t compass_event;
// Compass navigation
int targetHeading;              // where we want to go to reach current waypoint
int currentHeading;             // where we are actually facing now
int headingError;               // signed (+/-) difference between targetHeading and currentHeading
#define HEADING_TOLERANCE 5     // tolerance +/- (in degrees) within which we don't attempt to turn to intercept targetHeading

/** Ping sensor **/
#define TRIGGER_PIN 11
#define ECHO_PIN 11
#define MAX_DISTANCE_CM 250 //max dist we want to ping for
#define MAX_DISTANCE_IN (MAX_DISTANCE_CM/2.5) //max dist to ping for in inches
int sonarDistance;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE_CM);  //create ping object
MovingAverage<int, 3> sonarAverage(MAX_DISTANCE_IN);       // moving average of last n pings, initialize at MAX_DISTANCE_IN

/** GPS **/
#define GPSECHO false //for debugging
SoftwareSerial mySerial(8, 7);  //digital pins used with shield
Adafruit_GPS GPS(&mySerial);//GPS object
//default settings
boolean usingInterrupt = false;
void useInterrupt(boolean);
float currentLat,
      currentLong,
      targetLat,
      targetLong;
int distanceToTarget,            // current distance to target (current waypoint)
    originalDistanceToTarget;    // distance to original waypoing when we started navigating to it

   // Waypoints
#define WAYPOINT_DIST_TOLERANCE  2   // tolerance in meters to waypoint; once within this tolerance, will advance to the next waypoint
#define NUMBER_WAYPOINTS 5          // enter the numebr of way points here (will run from 0 to (n-1))
int waypointNumber = -1;            // current waypoint number; will run from 0 to (NUMBER_WAYPOINTS -1); start at -1 and gets initialized during setup()
waypointClass waypointList[NUMBER_WAYPOINTS] = {waypointClass(44.045808, -123.071446), waypointClass(44.045949, -123.071424), waypointClass(44.045726, -123.071513), waypointClass(44.046094, -123.071582), waypointClass(44.046083, -123.071705) };
/** Motors **/
//Create the motor shield objects
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

//Create our motors
Adafruit_DCMotor *leftMotor = AFMS.getMotor(2); //Using port M2
Adafruit_DCMotor *rightMotor = AFMS.getMotor(3); //Using port M3

#define TURN_LEFT 1
#define TURN_RIGHT 2
#define TURN_STRAIGHT 99

// Steering/turning 
enum directions {left = TURN_LEFT, right = TURN_RIGHT, straight = TURN_STRAIGHT} ;
directions turnDirection = straight;

// Object avoidance distances (in inches)
#define SAFE_DISTANCE 70
#define TURN_DISTANCE 40
#define STOP_DISTANCE 12

//Motor speeds range: 0-255
#define FAST_SPEED 250
#define NORMAL_SPEED 225
#define TURN_SPEED 150
#define SLOW_SPEED 100
int speed = NORMAL_SPEED;

/************* Setup *************/
void setup() {

  //Start motors
  AFMS.begin();

  //Set motor speed
  leftMotor->setSpeed(NORMAL_SPEED);
  rightMotor->setSpeed(NORMAL_SPEED);
  

  //Start compass
  if(!compass.begin())
  {
    loopForever();         // loop forever, can't operate without compass
  }

  //Start GPS
  GPS.begin(9600);                                // 9600 NMEA default speed
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     // turns on RMC and GGA (fix data)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);       // 1 Hz update rate
  GPS.sendCommand(PGCMD_NOANTENNA);                // turn off antenna status info
  useInterrupt(true);                            // use interrupt to constantly pull data from GPS
  delay(1000);

  //Wait for GPS to get signal
  
  unsigned long startTime = millis();
  while (!GPS.fix)                      // wait for fix, updating display with each new NMEA sentence received
  {
     // show how long we have waited  
    if (GPS.newNMEAreceived())
      GPS.parse(GPS.lastNMEA());      
  } // while (!GPS.fix)

  //Initiate countdown
  
  for (int i = 10; i > 0; i--)
  {
    if (GPS.newNMEAreceived())
         GPS.parse(GPS.lastNMEA());      
    delay(500);
  }

    waypointNumber = -1;   
   //Get initial waypoint
   nextWaypoint();
}


/************* Loop *************/
void loop() {
  // Process GPS 
    if (GPS.newNMEAreceived())               // check for updated GPS information
      {                                      
        if(GPS.parse(GPS.lastNMEA()) )      // if we successfully parse it, update our data fields
          processGPS();   
      } 
  
    // navigate 
    currentHeading = readCompass();    // get our current heading
    calcDesiredTurn();                // calculate how we would optimatally turn, without regard to obstacles      
    
    // distance in front of us, move, and avoid obstacles as necessary
    checkSonar();
    moveAndAvoid();  

    // update display and serial monitor    
    updateDisplay();   
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

//loopForever
void loopForever(void)
{
  while(1);
}

// Called after new GPS data is received; updates our position and course/distance to waypoint
void processGPS(void)
{
  currentLat = convertDegMinToDecDeg(GPS.latitude);
  currentLong = convertDegMinToDecDeg(GPS.longitude);
             
  if (GPS.lat == 'S')            // make them signed
    currentLat = -currentLat;
  if (GPS.lon = 'W')  
    currentLong = -currentLong; 
             
  // update the course and distance to waypoint based on our new position
  distanceToWaypoint();
  courseToWaypoint();         
  
}   // processGPS(void)




void checkSonar(void)
{   
  int dist;

  dist = sonar.ping_in();                   // get distqnce in inches from the sensor
  if (dist == 0)                                // if too far to measure, return max distance;
    dist = MAX_DISTANCE_IN;  
  sonarDistance = sonarAverage.add(dist);      // add the new value into moving average, use resulting average
} // checkSonar()




int readCompass(void)
{
  compass.getEvent(&compass_event);    
  float heading = atan2(compass_event.magnetic.y, compass_event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/ 
  // Cedar Park, TX: Magnetic declination: 4° 11' EAST (POSITIVE);  1 degreee = 0.0174532925 radians
  
  #define DEC_ANGLE 0.069
  heading += DEC_ANGLE;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  return ((int)headingDegrees); 
}  // readCompass()



void calcDesiredTurn(void)
{
    // calculate where we need to turn to head to destination
    headingError = targetHeading - currentHeading;
    
    // adjust for compass wrap
    if (headingError < -180)      
      headingError += 360;
    if (headingError > 180)
      headingError -= 360;
  
    // calculate which way to turn to intercept the targetHeading
    if (abs(headingError) <= HEADING_TOLERANCE)      // if within tolerance, don't turn
      turnDirection = straight;  
    else if (headingError < 0)
      turnDirection = left;
    else if (headingError > 0)
      turnDirection = right;
    else
      turnDirection = straight;
 
}  // calcDesiredTurn()




void moveAndAvoid(void)
{

    if (sonarDistance >= SAFE_DISTANCE)       // no close objects in front of car
        {
           if (turnDirection == straight)
             speed = FAST_SPEED;
           else
             speed = TURN_SPEED;
           leftMotor->setSpeed(speed);
           rightMotor->setSpeed(speed);
           //TODO!!!
           if (){

            
           }

            
           rightMotor->run(FORWARD);   
           leftMotor->run(FORWARD);    
           return;
        }
      
     if (sonarDistance > TURN_DISTANCE && sonarDistance < SAFE_DISTANCE)    // not yet time to turn, but slow down
       {
         if (turnDirection == straight)
           speed = NORMAL_SPEED;
         else
           {
              speed = TURN_SPEED;
              turnMotor->run(turnDirection);      // alraedy turning to navigate
            }
         driveMotor->setSpeed(speed);
         driveMotor->run(FORWARD);       
         return;
       }
     
     if (sonarDistance <  TURN_DISTANCE && sonarDistance > STOP_DISTANCE)  // getting close, time to turn to avoid object        
        {
          speed = SLOW_SPEED;
          driveMotor->setSpeed(speed);      // slow down
          driveMotor->run(FORWARD); 
          switch (turnDirection)
          {
            case straight:                  // going straight currently, so start new turn
              {
                if (headingError <= 0)
                  turnDirection = left;
                else
                  turnDirection = right;
                turnMotor->run(turnDirection);  // turn in the new direction
                break;
              }
            case left:                         // if already turning left, try right
              {
                turnMotor->run(TURN_RIGHT);    
                break;  
              }
            case right:                       // if already turning right, try left
              {
                turnMotor->run(TURN_LEFT);
                break;
              }
          } // end SWITCH
          
         return;
        }  


     if (sonarDistance <  STOP_DISTANCE)          // too close, stop and back up
       {
         driveMotor->run(RELEASE);            // stop 
         turnMotor->run(RELEASE);             // straighten up
         turnDirection = straight;
         driveMotor->setSpeed(NORMAL_SPEED);  // go back at higher speet
         driveMotor->run(BACKWARD);           
         while (sonarDistance < TURN_DISTANCE)       // backup until we get safe clearance
           {
              if(GPS.parse(GPS.lastNMEA()) )
                 processGPS();  
              currentHeading = readCompass();    // get our current heading
              calcDesiredTurn();                // calculate how we would optimatally turn, without regard to obstacles      
              checkSonar();
              updateDisplay();
              delay(100);
           } // while (sonarDistance < TURN_DISTANCE)
         driveMotor->run(RELEASE);        // stop backing up
         return;
        } // end of IF TOO CLOSE
     
}   // moveAndAvoid()
    




void nextWaypoint(void)
{
  waypointNumber++;
  targetLat = waypointList[waypointNumber].getLat();
  targetLong = waypointList[waypointNumber].getLong();

  /**
  if ((targetLat == 0 && targetLong == 0) || waypointNumber >= NUMBER_WAYPOINTS)    // last waypoint reached? 
    {
      driveMotor->run(RELEASE);    // make sure we stop
      turnMotor->run(RELEASE);  
      lcd.clear();
      lcd.println(F("* LAST WAYPOINT *"));
      loopForever();
    }
    **/
   processGPS();
   distanceToTarget = originalDistanceToTarget = distanceToWaypoint();
   courseToWaypoint();
   
}  // nextWaypoint()




// returns distance in meters between two positions, both specified 
// as signed decimal-degrees latitude and longitude. Uses great-circle 
// distance computation for hypothetical sphere of radius 6372795 meters.
// Because Earth is no exact sphere, rounding errors may be up to 0.5%.
// copied from TinyGPS library
int distanceToWaypoint() 
{
  
  float delta = radians(currentLong - targetLong);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  float lat1 = radians(currentLat);
  float lat2 = radians(targetLat);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong); 
  delta = sq(delta); 
  delta += sq(clat2 * sdlong); 
  delta = sqrt(delta); 
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong); 
  delta = atan2(delta, denom); 
  distanceToTarget =  delta * 6372795; 
   
  // check to see if we have reached the current waypoint
  if (distanceToTarget <= WAYPOINT_DIST_TOLERANCE)
    nextWaypoint();
    
  return distanceToTarget;
}  // distanceToWaypoint()




// returns course in degrees (North=0, West=270) from position 1 to position 2,
// both specified as signed decimal-degrees latitude and longitude.
// Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
// copied from TinyGPS library
int courseToWaypoint() 
{
  float dlon = radians(targetLong-currentLong);
  float cLat = radians(currentLat);
  float tLat = radians(targetLat);
  float a1 = sin(dlon) * cos(tLat);
  float a2 = sin(cLat) * cos(tLat) * cos(dlon);
  a2 = cos(cLat) * sin(tLat) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  targetHeading = degrees(a2);
  return targetHeading;
}   // courseToWaypoint()





// converts lat/long from Adafruit degree-minute format to decimal-degrees; requires <math.h> library
double convertDegMinToDecDeg (float degMin) 
{
  double min = 0.0;
  double decDeg = 0.0;
 
  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);
 
  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );
 
  return decDeg;
}

// Uses 4 line LCD display to show the following information:
// LINE 1: Target Heading; Current Heading;
// LINE 2: Heading Error; Distance to Waypoint; 
// LINE 3: Sonar Distance; Speed;
// LINE 4: Memory Availalble; Waypoint X of Y;  
void updateDisplay(void)
{


}  // updateDisplay()  

// Display free memory available
int freeRam ()   // display free memory (SRAM)
{
    extern int __heap_start, *__brkval; 
    int v; 
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
} // freeRam()


