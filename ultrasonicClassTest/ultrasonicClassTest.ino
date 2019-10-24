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

int counter;

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
  counter = 0;
 }
 
 void loop() {
   if (counter == 0) {
      counter++;
      UltrasonicClass ultrasonicClass;
      ultrasonicClass.positionServo(15);
   }
 }

/*
int angleIncrement, angleDirection, currentAngle, minAngle, maxAngle;
int distance, iterations, counter;
void setup() {
  minAngle = -45;
  maxAngle = 45;

  counter = iterations = 0;

  angleDirection = 1;
  angleIncrement = 1;
  currentAngle = minAngle;
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

void loop() {
  if (counter == 0) {
    counter++;
    positionServo(0);
    if (true == false) {
      showUltrasonic(90,distanceRight());
      delay(1000);
    }

    if (true == false) {
      showUltrasonic(-90,distanceLeft());
      delay(1000);
    }

    
    if (true == true) {
      showUltrasonic(0,distanceAtAngle(0));
      delay(1000);
    }

    
    // Test positioning
    if (true == false) {
      positionServo(ULTRASONIC_RIGHT_ANGLE);
      delay(3000);
      positionServo(ULTRASONIC_LEFT_ANGLE);
      delay(3000);
      positionServo(0);
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
        showUltrasonic(currentAngle,distanceAtAngle(currentAngle));
        //delay(100); // wait 0.1 seconds (100 milliseconds)
        currentAngle += ( angleIncrement * angleDirection);
      }
      positionServo(0);
    }
  } 
}
*/
