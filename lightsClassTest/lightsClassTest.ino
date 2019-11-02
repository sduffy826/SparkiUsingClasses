#include "lightsClass.h"

#define TESTLIGHTDELTAS false
#define TESTLIGHTS false
#define TESTLIGHTTURN false
#define TESTFINDLIGHTS false

#define LIGHTLOG true

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
  // Just want to test once :)
  if (counter == 0) {
    counter++;

    // For testing here we want pointers for the code below, test it with 
    UltrasonicClass ultrasonicObject;
    UltrasonicClass *ultrasonicObj;
    ultrasonicObj = &ultrasonicObject;
    
    LocalizationClass localizationObject;
    LocalizationClass *localizationObj;
    localizationObj = &localizationObject;
    
    localizationObj->setPose(10.0, 20.0, 0);

    DetermineWorldClass determineWorldObject(ultrasonicObject, localizationObject);
    DetermineWorldClass *determineWorldObj;
    determineWorldObj = &determineWorldObject;
    
    MovementsClass movementsObject(ultrasonicObject, localizationObject, determineWorldObject);
    MovementsClass *movementsObj;
    movementsObj = &movementsObject;
    

    LightsClass lightsObject(ultrasonicObject, localizationObject, movementsObject);
    LightsClass *lightsObj;
    lightsObj = &lightsObject;
    
    determineWorldObj->calculateRectangularCoordinates();
    determineWorldObj->showWorld();
    
    // Make sure the servo isn't blocking a light
    ultrasonicObj->positionServo(0);

    #if TESTLIGHTTURN
      // localizationObj->setPose(1.0, 2.0, 0);
      movementsObj->turnToZero();
      sparki.beep();
      Serial.println("After first turnToZero");
      delay(2000);
      movementsObj->turnToAngle(0);
      sparki.beep();
      Serial.println("After second turnToZero");
      delay(2000);
      localizationObj->writeMsg2Serial("End");
    #endif

    #if TESTLIGHTS
      // Calculate world
      localizationObj->writeMsg2Serial("LightsOff");
      determineWorldObj->calculateRectangularCoordinates();
    
      lightsObj->sampleWorldLights();
      sparki.beep();
      localizationObj->writeMsg2Serial("LightsOn");
      delay(5000);
      lightsObj->setPotentialLightTargets();
    #endif
    
    #if TESTLIGHTDELTAS
      // Test this it aint working as expected
      localizationObj->writeMsg2Serial("LiteOff");
      determineWorldObj->calculateRectangularCoordinates();
    
      lightsObj->sampleWorldLights();
      sparki.beep();
      localizationObj->writeMsg2Serial("LiteOn");
      delay(5000);

      localizationObj->writeMsg2Serial("calcDlta");
      
      lightsObj->calculateWorldLightDeltas();
      int theAngle = lightsObj->getAngleWithHighestLightDelta(-1,-1);  // Use invalid angle to not ignore
      lightsObj->showCalibrationLightAtAngle(theAngle);
      // put in line above instead of this.. LOOK INTO if that's what was wanted lightsObj->showLightDirection(theAngle);

      // Get the next brightest
      theAngle = lightsObj->getAngleWithHighestLightDelta(localizationObj->getAngle(theAngle-90),localizationObj->getAngle(theAngle+90));
      // Had this, I changed it to method below... not sure if that's what was wanted... LOOK INTO lightsObj->showLightDirection(theAngle);
      lightsObj->showCalibrationLightAtAngle(theAngle);
      localizationObj->writeMsg2Serial("dun");
    #endif

    #define TESTL10 false
    #if TESTL10
      localizationObj->writeMsg2Serial("Delta pct between 116 and 65");
      Serial.println(lightsObj->getLightDeltaPctBetween2Values(116,65));
    #endif

    #define TESTL20 false
    #if TESTL20
      // Helper method
      if (lightsObj->numberBetweenRange(30,210,30)) {
        localizationObj->writeMsg2Serial("0 is between 210 and 30");
      }
      else {
        localizationObj->writeMsg2Serial("0 is NOT between 210 and 30");
      }
    #endif

    #define TESTL30 false
    #if TESTL30
      Pose robotPose = localizationObj->getPose();
      movementsObj->turnLeft(45);
      while (movementsObj->moveForward(10.0,5,false));
      determineWorldObj->showWorld();
      localizationObj->showPose(robotPose);  // Show original pose
      localizationObj->showPose(localizationObj->getPose());  // Show ending pose
    #endif


    #if TESTFINDLIGHTS
      LightDeltaAmounts deltaAmts;
      lightsObj->sampleWorldLights();

      sparki.beep();
      localizationObj->writeMsg2Serial("LiteOn");
      delay(5000);

      localizationObj->writeMsg2Serial("calcDlta");
      lightsObj->calculateWorldLightDeltas();
      
      int angleVar1 = -1;
      int angleVar2 = -1;
      int MAXAMOUNT = 999;
      int MAXDELTA = 4;
      int lastAngleTurned = 360;
      Pose robotPose = localizationObj->getPose();
      for (int iteration = 0; iteration < 2; iteration++) {
        
        int angle2GoTo = lightsObj->getAngleWithBrightestCurrentLight(angleVar1, angleVar2, 5);
        
        #if LIGHTLOG
          Serial.print("<Ig1,");
          Serial.print(angleVar1);
          Serial.print(",<Ig2,");
          Serial.print(angleVar2);
          Serial.print(",Lght<2GoTo,");
          Serial.println(angle2GoTo);
        #endif
        
        movementsObj->turnToAngle(angle2GoTo);
        // Save the original light attributes
        LightAttributes originalLightAttributes = lightsObj->getLightAttributesAtCurrentPose();
        // Clear all your delta's
        lightsObj->clearLightsDeltaSum(deltaAmts);
                
        #if LIGHTLOG
          lightsObj->showLightAttributes("Orig",originalLightAttributes,angle2GoTo);
        #endif
        
        LightAttributes currentLightAttributes;
        bool stopMoving = false;
        bool done = false;
        while ( (movementsObj->moveForward(999,ULTRASONIC_MIN_SAFE_DISTANCE,true) == true) && (done == false) ) {
          delay(100);
          
          currentLightAttributes = lightsObj->getLightAttributesAtCurrentPose();
          #if LIGHTLOG
            lightsObj->showLightAttributes("Curr",currentLightAttributes,localizationObj->getCurrentAngle());
          #endif
 
          if (currentLightAttributes.lightCenter >= MAXAMOUNT) {
            // At target turn and go other way
            done = true;
            localizationObj->writeMsg2Serial("Cntr>MAXAMT");
          }
          else {
            // Calculate to see if we should adjust our angle
            angleVar1 = lightsObj->getLightAngleToTurnTo(deltaAmts, originalLightAttributes, currentLightAttributes);
            if (angleVar1 != 0) {
              // We have an angle to turn to, so set stopMoving to true, we'll do the turning down there
              stopMoving = true;
              if (lastAngleTurned == angleVar1) {
                angleVar1 *= 2; // If it's same angle to turn as last time then move twice the amount in that direction.
              }
              else {
                lastAngleTurned = angleVar1;
              }
              
              #if LIGHTLOG
                Serial.print("<Chg");
                Serial.println(angleVar1);
              #endif
            }
            else {
              if ( ((deltaAmts.lightCenter) < 0) && (deltaAmts.centerIncCnt <= -LIGHTDELTAS2ACTON) ) {
                // It's consistently getting darker, stop and go other way.
                localizationObj->writeMsg2Serial("LghtWent-");
                done = true;
              }
            }
          }
          
          if ((stopMoving == true) || (done == true)) {
            movementsObj->stopMoving();
            delay(200);
            
            if (stopMoving == true) {
              // We need to turn
              stopMoving = false;
              if (angleVar1 < 0) {
                movementsObj->turnLeft(-angleVar1);
              }
              else {
                movementsObj->turnRight(angleVar1);
              }
              delay(100);
              // Use the current light settings as your new originals
              // dont want this, originalLightAttributes = lightsObj->getLightAttributesAtCurrentPose();
            }
            else {
              // We're done
              // Calculate the angle we traveled to get from start to our current position
              localizationObj->showPose(robotPose);
              localizationObj->showPose(localizationObj->getPose());
              angleVar1 = localizationObj->calculateAngleBetweenPoints(robotPose.xPos, robotPose.yPos, localizationObj->getCurrentXPosition(), localizationObj->getCurrentYPosition());
              #if LIGHTLOG
                Serial.print("< Traveled");
                Serial.println(angleVar1);
              #endif
              
              // set angleVar values that will be ignored on next iteration
              angleVar2 = localizationObj->getAngle(angleVar1+60);
              angleVar1 = localizationObj->getAngle(angleVar1-60);
 
               // Go back to the original spot
              movementsObj->moveToPose(robotPose);
              #if LIGHTLOG
                Serial.print("BackAtStrg");
                localizationObj->showLocation();
              #endif
           
              delay(500);
              sparki.beep();
            }
            
            lightsObj->clearLightsDeltaSum(deltaAmts); 

            #if LIGHTLOG
              lightsObj->showLightsDeltaSum(deltaAmts);
            #endif
            
          }
        }
      }
    #endif
  }
}
