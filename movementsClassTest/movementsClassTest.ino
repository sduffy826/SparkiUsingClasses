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
    UltrasonicClass ultrasonicObj;
    LocalizationClass localizationObj;
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
    //determineWorldObj.adjustWorldCoordinate(100.0,200.0);  // Just for testing :)
    
    MovementsClass movementObj(ultrasonicObj, localizationObj, determineWorldObj);

    if (true == false) {
      movementObj.initMovements();
      movementObj.turnRight(90); 
      int theDistance = ultrasonicObj.distanceAtAngle(0);
      while (movementObj.moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      
      theDistance = ultrasonicObj.distanceAtAngle(0);
      Serial.print("Forward distance: ");
      Serial.println(theDistance);
      
      movementObj.turnLeft(90);
      theDistance = ultrasonicObj.distanceAtAngle(90);
      Serial.print("Distance from right distance: ");
      Serial.println(theDistance);
    }    
    
    if (true == true) {
      // routine below has the logic to calculate our rectangular world coordinates
      movementObj.initMovements();
      movementObj.followWall();
    }

    if (true == true) {
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
    
    counter++;
  }
}
