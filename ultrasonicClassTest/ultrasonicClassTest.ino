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
UltrasonicClass *ultrasonicPtr;

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
    ultrasonicPtr = &ultrasonicObj;

    LocalizationClass localizationObj;
    localizationObj.setPose(1.0, 2.0, 3.0);
       
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
   
    ultrasonicPtr->setDetermineWorldObj(determineWorldObj);
    ultrasonicPtr->setLocalizationObj(localizationObj);

    if (true == true) {
      for (int idx = -85; idx < 85; idx++) {
      // for (int idx = 20; idx > -20; idx--) {
        ultrasonicPtr->positionServo(idx);
        delay(50);
        ultrasonicPtr->getDistanceFromServoAtAngle(ultrasonicPtr->getServoAngle());  
      }
      ultrasonicPtr->positionServo(0);
      
    }

    if (true == false) {
      Serial.println("Start program");
      delay(5000);
      
      ultrasonicPtr->positionServo(0);
      Serial.println("getDistanceFromServoAtAngle");
      ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getDistanceFromServoAtAngle(ultrasonicPtr->getServoAngle()));

      Serial.println("getFreeSpaceInFrontOfGripper");
      ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getFreeSpaceInFrontOfGripper(ultrasonicPtr->getServoAngle()));

      Serial.println("getFreeSpaceInFrontExcludingGripper");
      ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getFreeSpaceInFrontExcludingGripper(ultrasonicPtr->getServoAngle()));

      Serial.println("getDistanceFromCenterOfRobotToObstacle");
      ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getDistanceFromCenterOfRobotToObstacle(ultrasonicPtr->getServoAngle()));

      Serial.println("getFreeSpaceOnRight");
      ultrasonicPtr->showUltrasonic(90,ultrasonicPtr->getFreeSpaceOnRight());

      Serial.println("getFreeSpaceOnLeft");
      ultrasonicPtr->showUltrasonic(-90,ultrasonicPtr->getFreeSpaceOnLeft());

      Serial.print("getSensorTolerance");
      Serial.println(ultrasonicPtr->getSensorTolerance());
    }
  
    if (true == false) {
      ultrasonicObj.positionServo(80);
      ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getDistanceFromServoAtAngle(ultrasonicPtr->getServoAngle()));
   }
    
    
    if (true == false) {
      ultrasonicObj.showUltrasonic(-90,ultrasonicObj.getFreeSpaceOnLeft());
      delay(1000);
    }
    
    if (true == false) {
      ultrasonicObj.showUltrasonic(0,ultrasonicObj.getDistanceFromServoAtAngle(0));
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
      // Calculate angle between -90 and 90, this will also show the distance from the center of the robot to the obstacle
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
        ultrasonicObj.showUltrasonic(currentAngle,ultrasonicObj.getDistanceFromServoAtAngle(currentAngle));
        
        // Now show the real distance to the center of the bot
        ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getDistanceFromCenterOfRobotToObstacle(ultrasonicPtr->getServoAngle()));


        //delay(100); // wait 0.1 seconds (100 milliseconds)
        currentAngle += ( angleIncrement * angleDirection);
      }
      ultrasonicObj.positionServo(0);
     }

     #define SHOWULTRADETADDR false
     #if SHOWULTRADETADDR  // This is for showing address and how to update a class with a reference to another object
       // Here it should be null
       ultrasonicPtr->showDetermineWorldAddr();
       
       LocalizationClass localizationObj;
       localizationObj.setPose(1.0, 2.0, 3.0);
       
       DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
       DetermineWorldClass *detPtr = &determineWorldObj;
       determineWorldObj.showWorld();

       // Show the address of the determineWorldObj
       unsigned long detWorld = (long)detPtr;
       Serial.print("detPtr addr: ");
       Serial.println(detWorld);

       // How pass the determineWorldObj into the setDetermineWorldObj method so it has a reference
       // to determineWorldObj :) then have it output the address of that object
       //ultrasonicPtr->setDetermineWorldObj(determineWorldObj);
       ultrasonicPtr->showDetermineWorldAddr();
     #endif
     
   }
   
   //ultrasonicPtr->showUltrasonic(ultrasonicPtr->getServoAngle(),ultrasonicPtr->getDistanceFromServoAtAngle(ultrasonicPtr->getServoAngle()));
   delay(500);
  
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
