#include "lightsClass.h"

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
    localizationObj.setPose(1.0, 2.0, 3);
    
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
    MovementsClass movementsObj(ultrasonicObj, localizationObj, determineWorldObj);

    LightsClass lightsObj(ultrasonicObj, localizationObj, movementsObj);
    
    // Make sure the servo isn't blocking a light
    ultrasonicObj.positionServo(0);

    if (true == true) {
      // localizationObj.setPose(1.0, 2.0, 0);
      movementsObj.turnToZero();
      sparki.beep();
      Serial.println("After first turnToZero");
      delay(2000);
      movementsObj.turnToAngle(0);
      sparki.beep();
      Serial.println("After second turnToZero");
      delay(2000);
      localizationObj.writeMsg2Serial("End");
    }
    if (true == false) {
    // Calculate world
    localizationObj.writeMsg2Serial("LightsOff");
    determineWorldObj.calculateRectangularCoordinates();
    
    lightsObj.sampleWorldLights();
    sparki.beep();
    localizationObj.writeMsg2Serial("LightsOn");
    delay(5000);
    }
    if (true == false) {
      lightsObj.setPotentialLightTargets();
    }
    
    if (true == false) {
    
      // Test this it aint working as expected
      lightsObj.calculateLightDeltas();
      int theAngle = lightsObj.getAngleWithHighestLightDelta(-1,-1);  // Use invalid angle to not ignore
      lightsObj.showLightDirection(theAngle);

      // Get the next brightest
      theAngle = lightsObj.getAngleWithHighestLightDelta(localizationObj.getAngle(theAngle-90),localizationObj.getAngle(theAngle+90));
      lightsObj.showLightDirection(theAngle);
    } 
   
  }
}
