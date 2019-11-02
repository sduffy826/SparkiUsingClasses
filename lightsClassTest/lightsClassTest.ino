#include "lightsClass.h"

#define TESTLIGHTDELTAS false
#define TESTLIGHTS false
#define TESTLIGHTTURN false
#define TESTFINDLIGHTS true

#define LIGHTDELTAS2ACTON 3  // We want three positive or negative delta's before we take action
#define LIGHTDELTAANGLE2TURN 5

#define LIGHTLOG true

byte counter = 0;
struct LightDeltaAmounts {
  unsigned int lightLeft : 12;
  unsigned int leftIncCnt : 4;  // Counter of how many times this value increased
  unsigned int lightCenter : 12;
  unsigned int centerIncCnt : 4;
  unsigned int lightRight : 12;
  unsigned int rightIncCnt : 4;
};
     
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


void clearDeltaAmounts(LightDeltaAmounts &amts) {
  showDeltaAmounts(amts);
  amts.lightLeft = 0;
  amts.leftIncCnt = 0;
  amts.lightCenter = 0;
  amts.centerIncCnt = 0;
  amts.lightRight = 0;
  amts.rightIncCnt = 0;
}

void showDeltaAmounts(const LightDeltaAmounts &amts) {
  Serial.print("LDA,l");
  Serial.print(amts.lightLeft);
  Serial.print(",li,");
  Serial.print(amts.leftIncCnt);
  Serial.print(",c,");
  Serial.print(amts.lightCenter);
  Serial.print(",ci,");
  Serial.print(amts.centerIncCnt);
  Serial.print(",r,");
  Serial.print(amts.lightRight);
  Serial.print(",ri,");
  Serial.println(amts.rightIncCnt);
}

void setDeltaAmounts(LightDeltaAmounts &amts, const LightAttributes &original, const LightAttributes &current) {
  amts.lightLeft += (current.lightLeft - original.lightLeft);
  amts.leftIncCnt += (current.lightLeft > original.lightLeft ? 1 : (current.lightLeft < original.lightLeft ? -1 : 0));
  
  amts.lightCenter += (current.lightCenter - original.lightCenter);
  amts.centerIncCnt += (current.lightCenter > original.lightCenter ? 1 : (current.lightCenter < original.lightCenter ? -1 : 0));
  
  amts.lightRight += (current.lightRight - original.lightRight);
  amts.rightIncCnt += (current.lightRight > original.lightRight ? 1 : (current.lightRight < original.lightRight ? -1 : 0));
}

// This returns the id of the light that has the larger count (has delta counts), if they match it'll return the one with the highest delta value, if 
// we don't find a light to look at we'll return -1 (don't use that as a light id).
int lightDeltaAmountsHelper(const int &lightDelta1, const int &lightCnt1, const byte &lightId1, const int &lightDelta2, const int &lightCnt2, const byte &lightId2 ) {
  if ((lightCnt1 >= LIGHTDELTAS2ACTON) || (lightCnt2 >= LIGHTDELTAS2ACTON)) {  // At least one of the lights is above threshold
    if ((lightCnt1 - lightCnt2) == 0) {
      // Same delta count return id of the larger delta
      return (lightDelta1 > lightDelta2 ? lightId1 : lightId2);
    }
    else {
      return (lightCnt1 > lightCnt2 ? lightId1 : lightId2);
    }
  }
  else
    return -1;
}

// Little helper to compare left, center and right lights and return
int getAngleToTurn(LightDeltaAmounts &amts, const LightAttributes &original, const LightAttributes &current) {
  setDeltaAmounts(amts, original, current);
  // Get id of lights that are more significant
  int idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightLeft, amts.leftIncCnt, 1, amts.lightCenter, amts.centerIncCnt, 2);
  if (idOfLight2Point2 = 1) { // Left is significant, compare it to the right light
    idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightLeft, amts.leftIncCnt, 1, amts.lightRight, amts.rightIncCnt, 3);
  }
  else { // Center is more significant of the left light, compare it to the right.
    idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightCenter, amts.centerIncCnt, 2,amts.lightRight, amts.rightIncCnt, 3);
  }
  if (idOfLight2Point2 > 0) {
    // One is signifcant
    return ((idOfLight2Point2 - 2) * LIGHTDELTAANGLE2TURN);  // so this will return -LIGHTDELTAANGLE2TURN, 0 or LIGHTDELTAANGLETOTURN (for left, center, right)
  }
  return 0;
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
      
      lightsObj->calculateLightDeltas();
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
      lightsObj->calculateLightDeltas();
      
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
        clearDeltaAmounts(deltaAmts);
                
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
            angleVar1 = getAngleToTurn(deltaAmts, originalLightAttributes, currentLightAttributes);
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
            
            clearDeltaAmounts(deltaAmts); 

            #if LIGHTLOG
              showDeltaAmounts(deltaAmts);
            #endif
            
          }
        }
      }
    #endif
  }
}
