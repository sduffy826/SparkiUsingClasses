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
    counter++;
    LocalizationClass localizationObj;
    localizationObj.setPose(20.0, 4.0, 179);
    
    if (true == false) {
      // Test that the turn angles are correct
      localizationObj.setPose(20.0, 4.0, 0);
      // Show another call, we'll move do it another way
      //localizationObj.setNewPosition(4.9, 20.0);  
      //localizationObj.showLocation();
  
      for (int i = 0; i < 360; i+=15) {
        Serial.print("from: ");
        Serial.print(localizationObj.getCurrentAngle());
        Serial.print(" to: ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.println(localizationObj.getShortestAngleDeltaToGetToOrientation(i));    
        Serial.println(localizationObj.getSlopeOfAngle(i));
        //Serial.print("Tan: ");
        //Serial.println(tan( ((float)i/10.0)*(3.14159/180.0))); //      localizationObj.degreesToRadians((float)i/10.0)));
      }
    }
    
    if (true == true) {
      // Test triangulation
      Pose startPose;
      Pose endPose;

      startPose.xPos = 0.0;
      startPose.yPos = 0.0;
      startPose.angle = 45;

      endPose.xPos = 10.0;
      endPose.yPos = 0;
      endPose.angle = 315;

      localizationObj.showPose(localizationObj.getPointOfIntersection(startPose, endPose));
      /*
      Pose triangulationPose = localizationObj.triangulatePoses(startPose, endPose, 0);

      localizationObj.showPose(startPose);
      localizationObj.showPose(endPose);
      localizationObj.showPose(triangulationPose);
      */
    }
  
  
  
  }
}




