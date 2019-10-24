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

    UltrasonicClass ultrasonicObj;
    LocalizationClass localizationObj;
    localizationObj.setPose(1.0, 2.0, 3.0);
    
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
    MovementsClass movementsObj(ultrasonicObj, localizationObj, determineWorldObj);

    LightsClass lightsObj(ultrasonicObj, localizationObj, movementsObj);
    
    // Make sure the servo isn't blocking a light
    ultrasonicObj.positionServo(0);
  
    // Calculate world
    determineWorldObj.calculateRectangularCoordinates();
    
    lightsObj.sampleWorldLights();
    sparki.beep();
    delay(2000);

    if (true == true) {
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
    counter++;
  }
}
