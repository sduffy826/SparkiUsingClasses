#include "determineWorldClass.h"

int counter;

void setup() {
  counter = 0;
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
}

void loop() {
  // Just want to test once :)
  if (counter == 0) {
    counter++;
    UltrasonicClass ultrasonicObj;
    LocalizationClass localizationObj;
    localizationObj.setPose(1.0, 2.0, 3.0);
    DetermineWorldClass myWorld(ultrasonicObj, localizationObj);
    myWorld.showWorld();
    // routine below has the logic to calculate our rectangular world coordinates
    //myWorld.calculateRectangularCoordinates();
    
 
  }
}
