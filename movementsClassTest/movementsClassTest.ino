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
    determineWorldObj.adjustWorldCoordinate(100.0,200.0);  // Just for testing :)
    
    MovementsClass movementObj(ultrasonicObj, localizationObj, determineWorldObj);
    
    if (true == false) {
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

    if (true == true) {
      movementObj.turnRight(134);
      delay(2000);
      movementObj.turnTo90ClosestDegreeOrientation();
      delay(2000);
      movementObj.turnToZero();
    }
    
    counter++;
  }
}
