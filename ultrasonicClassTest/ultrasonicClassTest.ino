/*******************************************
 Basic Ultrasonic test
 
 Show the distance Sparki's eyes are reading
 on the LCD. Sparki will beep when something
 is too close. If it measures -1, that means 
 the sensor is either too close or too far 
 from an object
********************************************/
#include <Sparki.h> // include the robot library
#include "ultrasonicClass.h"

int distance, iterations, counter;
int angleIncrement, angleDirection, currentAngle, minAngle, maxAngle;

void setup() {
 // put your setup code here, to run once:
 #if USE_LCD 
   sparki.clearLCD();
   delay(500); 
 #else
   Serial.begin(SERIAL_SPEED);
   sparki.beep();
   delay(DELAY_AFTER_SERIAL_STARTUP);  
   sparki.beep();
 #endif
 minAngle = -45;
 maxAngle = 45;

 counter = iterations = 0;

 angleDirection = 1;
 angleIncrement = 1;
 currentAngle = minAngle;

}
 
void loop() {
  if (counter == 0) {
    counter++;
    UltrasonicClass ultrasonicObj;
  
    if (true == false) {
      ultrasonicObj.positionServo(15);
    }
    
    
    if (true == false) {
      ultrasonicObj.showUltrasonic(-90,ultrasonicObj.distanceLeft());
      delay(1000);
    }
    
    if (true == false) {
      ultrasonicObj.showUltrasonic(0,ultrasonicObj.distanceAtAngle(0));
      delay(1000);
    }
    
   
    // Test positioning
    if (true == false) {
      ultrasonicObj.positionServo(ULTRASONIC_RIGHT_ANGLE);
      delay(3000);
      ultrasonicObj.positionServo(ULTRASONIC_LEFT_ANGLE);
      delay(3000);
      ultrasonicObj.positionServo(0);
      delay(5000);
    }
    
    
    if (true == false) {
      // Calculate angle between -90 and 90
      while (iterations < 2) {
        if (currentAngle <= minAngle) {
          angleDirection = 1;
          iterations++;
        }
        else 
          if (currentAngle >= maxAngle ) {
            angleDirection = -1;
            iterations++;
          }
        ultrasonicObj.showUltrasonic(currentAngle,ultrasonicObj.distanceAtAngle(currentAngle));
        //delay(100); // wait 0.1 seconds (100 milliseconds)
        currentAngle += ( angleIncrement * angleDirection);
      }
      ultrasonicObj.positionServo(0);
    }
   }
  
 }

/*
int distance, iterations, counter;
void setup() {
  #if USE_LCD 
    sparki.clearLCD();
    delay(500);
  #else
    Serial.begin(SERIAL_SPEED);
    sparki.beep();
    delay(DELAY_AFTER_SERIAL_STARTUP);  
    sparki.beep();
  #endif
}

*/
