#include "movementsClass.h"

byte counter;

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
  // Just want to test once :)
  if (counter == 0) {
      
    counter++;
    
    UltrasonicClass ultrasonicObj;
    LocalizationClass localizationObj;
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
    //determineWorldObj.adjustWorldCoordinate(100.0,200.0);  // Just for testing :)
    
    MovementsClass movementObj(ultrasonicObj, localizationObj, determineWorldObj);

    if (true == false) {
      movementObj.initMovements();

      for (int i = 0; i < 20; i++) {
        int theDistance = ultrasonicObj.distanceAtAngle(90);
        Serial.print("Distance from right distance: ");
        Serial.println(theDistance);
        delay(500);
      }
    }


    if (true == false) {
      movementObj.initMovements();
      movementObj.turnRight(90); 
      int theDistance = ultrasonicObj.distanceAtAngle(0)+2.0;
      while (movementObj.moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      
      theDistance = ultrasonicObj.distanceAtAngle(0);
      Serial.print("Forward distance: ");
      Serial.println(theDistance);
      
      movementObj.turnLeft(90);
      theDistance = ultrasonicObj.distanceAtAngle(90);
      Serial.print("Distance from right distance: ");
      Serial.println(theDistance);
    }    

    if (true == false) {
      // Test moving, we save original spot, move some distance away and then return to it
      movementObj.initMovements();
      Pose originalSpot = localizationObj.getPose();
      
      localizationObj.showPose(originalSpot);
      
      // Move 5cm, turn, move 2 more, then return
      while (movementObj.moveForward(15.0, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      movementObj.turnLeft(90);
      while (movementObj.moveForward(7.0, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      
      localizationObj.showLocation();
      
      movementObj.moveToPose(originalSpot);
      localizationObj.showLocation();
      
    }
    if (true == true) {
      // routine below has the logic to calculate our rectangular world coordinates
      movementObj.initMovements();
      movementObj.followWall();
    }

    if (true == false) {
      movementObj.showTurnRadius();
    }

    if (true == false) {
      movementObj.turnRight(90);
    }

    if (true == false) {
      movementObj.turnRight(134);
      delay(2000);
      movementObj.turnTo90ClosestDegreeOrientation();
      delay(2000);
      movementObj.turnToZero();
    }
  }
}
