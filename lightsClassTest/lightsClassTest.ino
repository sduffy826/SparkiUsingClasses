#include "lightsClass.h"

#define TESTLIGHTDELTAS false
#define TESTLIGHTS false
#define TESTLIGHTTURN false

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

    #if TESTLIGHTTURN
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
    #endif

    #if TESTLIGHTS
    
      // Calculate world
      localizationObj.writeMsg2Serial("LightsOff");
      determineWorldObj.calculateRectangularCoordinates();
    
      lightsObj.sampleWorldLights();
      sparki.beep();
      localizationObj.writeMsg2Serial("LightsOn");
      delay(5000);
      lightsObj.setPotentialLightTargets();
      
    #endif
    
    #if TESTLIGHTDELTAS
      // Test this it aint working as expected
      localizationObj.writeMsg2Serial("LiteOff");
      determineWorldObj.calculateRectangularCoordinates();
    
      lightsObj.sampleWorldLights();
      sparki.beep();
      localizationObj.writeMsg2Serial("LiteOn");
      delay(5000);

      localizationObj.writeMsg2Serial("calcDlta");
      
      lightsObj.calculateLightDeltas();
      int theAngle = lightsObj.getAngleWithHighestLightDelta(-1,-1);  // Use invalid angle to not ignore
      lightsObj.showLightDirection(theAngle);

      // Get the next brightest
      theAngle = lightsObj.getAngleWithHighestLightDelta(localizationObj.getAngle(theAngle-90),localizationObj.getAngle(theAngle+90));
      lightsObj.showLightDirection(theAngle);
      localizationObj.writeMsg2Serial("dun");
    #endif

    if (true == false) {
      localizationObj.writeMsg2Serial("Delta pct between 116 and 65");
      Serial.println(lightsObj.getDeltaPct(116,65));
    }

    if (true == false) {
      // Helper method
      if (lightsObj.numberBetweenRange(30,210,30)) {
        localizationObj.writeMsg2Serial("0 is between 210 and 30");
      }
      else {
        localizationObj.writeMsg2Serial("0 is NOT between 210 and 30");
      }
    }

    if (true == true) {
      int angle2ignore1 = -1;
      int angle2ignore2 = -1;
      int MAXAMOUNT = 950;
      int MAXDELTA = 10;
      Pose robotPose = localizationObj.getPose();
      for (int iteration = 0; iteration < 2; iteration++) {
        int angle2GoTo = lightsObj.getAngleWithBrightestLight(angle2ignore1, angle2ignore2, 5);

        localizationObj.writeMsg2Serial("angle2GoTo");
        Serial.println(angle2GoTo);
        
        // set angle2ignore1 for next run
        angle2ignore1 = localizationObj.getAngle(angle2GoTo-60);
        angle2ignore2 = localizationObj.getAngle(angle2GoTo+60);
        movementsObj.turnToAngle(angle2GoTo);
        LightAttributes originalLightAttributes = lightsObj.getCurrentLightAttributes();
        
        localizationObj.writeMsg2Serial("Original light attributes");
        Serial.println(originalLightAttributes.lightCenter);
        
        LightAttributes currentLightAttributes;
        bool stopMoving = false;
        while (movementsObj.moveForward(999, ULTRASONIC_MIN_SAFE_DISTANCE, true) && stopMoving == false) {
          delay(200);
          currentLightAttributes = lightsObj.getCurrentLightAttributes();
          localizationObj.writeMsg2Serial("Current light attributes");
          Serial.println(currentLightAttributes.lightCenter);
          if (currentLightAttributes.lightCenter >= MAXAMOUNT) {
            // At target turn and go other way
            stopMoving = true;
          }
          else {
            int deltaOnCenter = currentLightAttributes.lightCenter - originalLightAttributes.lightCenter;
            // Could be near obstacle or we passed light, for now stop
            if (deltaOnCenter <= MAXDELTA) {
              stopMoving = true;
            }
          }
          if (stopMoving) {
             movementsObj.stopMoving();
             delay(500);
             movementsObj.moveToPose(robotPose);
             delay(500);
             sparki.beep();
          }
        }
      }
    }

  
  }
}
