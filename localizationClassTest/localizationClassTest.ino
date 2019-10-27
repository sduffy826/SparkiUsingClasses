#include <Sparki.h>
#include "localizationClass.h"

// Little test program to show localization routines, the meat is in the
// include file (localizationCommon.h)

byte counter = 0;

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
  // put your main code here, to run repeatedly:
  if (counter == 0) {

    LocalizationClass localizationObj;

    
    if (true == true) {
      // Test that the turn angles are correct
      localizationObj.setPose(20.0, 4.0, 179);
      // Show another call, we'll move do it another way
      //localizationObj.setNewPosition(4.9, 20.0);  
      //localizationObj.showLocation();
  
      for (int i = 0; i < 360; i+=30) {
        Serial.print("from: ");
        Serial.print(localizationObj.getCurrentAngle());
        Serial.print(" to: ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.println(localizationObj.getShortestAngleDeltaToGetToOrientation(i));    
      }
      
    }
    counter++;
  }
}




