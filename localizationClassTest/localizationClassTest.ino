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

    // Show conversion of degrees to radians and get slope of lines
    if (true == false) {
      int angle1 = 45;
      int angle2 = 30;
      Serial.print("Degrees 2 Radians (");
      Serial.print(angle1);
      Serial.print("): ");
      Serial.println(localizationObj.degreesToRadians(angle1));
      Serial.print("Slope: ");
      Serial.println(localizationObj.getSlopeOfAngle(angle1));
      
      Serial.print("Degrees 2 Radians (");
      Serial.print(angle2);
      Serial.print("): ");
      Serial.println(localizationObj.degreesToRadians(angle2));
      Serial.print("Slope: ");
      Serial.println(localizationObj.getSlopeOfAngle(angle2));
    }

    // Show angle between two points
    if (true == true) {
      float x1 = -2;
      float y1 = 1;
      //float x2 = 2.73;
      //float y2 = 2.74;
      float x2 = 2;
      float y2 = -1;          // 1.15 + .58;
      Serial.print("Angle between points ");
      Serial.print(x1);
      Serial.print(",");
      Serial.print(y1);
      Serial.print(" and ");
      Serial.print(x2);
      Serial.print(",");
      Serial.print(y2);
      Serial.print(" is: ");
      Serial.println(localizationObj.calculateAngleBetweenPoints(x1,y1,x2,y2));
      Serial.println(localizationObj.convertCoordinateSystemAngle(localizationObj.calculateAngleBetweenPoints(x1,y1,x2,y2)));
    }

    
    // Test triangulation
    if (true == true) {
      // Test triangulation
      Pose startPose;
      Pose endPose;

      startPose.xPos = 2.0;
      startPose.yPos = 1.0;
      startPose.angle = 180;

      endPose.xPos = 2.0;
      endPose.yPos = 2.0;
      endPose.angle = 185;

      Pose poseOfIntersection;
      if (localizationObj.setPointOfIntersection(startPose, endPose, poseOfIntersection) == false) {
        Serial.println("There is no point of intersection");
      }
      else {
        Serial.println("Point of intersection is at: ");
        localizationObj.showPose(poseOfIntersection);
      }
      /*
      Pose triangulationPose = localizationObj.triangulatePoses(startPose, endPose, 0);

      localizationObj.showPose(startPose);
      localizationObj.showPose(endPose);
      localizationObj.showPose(triangulationPose);
      */
    }
  
  
  
  }
}




