#include "lightsClass.h"

LightsClass::LightsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, MovementsClass &movementsObject) {
  ultrasonicObj = ultrasonicObject;
  localizationObj = localizationObject;
  movementsObj  = movementsObject;
}


// Gets the light attributes, we take samples and will sort them at the end, the program will use
// a constant (LIGHTSAMPLEVALUE2USE) to determine the value to use (median at time of this writing).
void LightsClass::setLightAttributes() {
  for (byte i = 0; i < LIGHTSAMPLESIZE; i++) {
    lightSample[i].lightLeft = sparki.lightLeft();
    lightSample[i].lightCenter = sparki.lightCenter();
    lightSample[i].lightRight = sparki.lightRight();
    delay(LIGHTSAMPLEDELAY);
  }
  // Sort arrays, simple bubble sort
  for (byte i = 0; i < LIGHTSAMPLESIZE-1; i++) {
    for (byte j = i + 1; j < LIGHTSAMPLESIZE; j++) {
      if (lightSample[i].lightLeft > lightSample[j].lightLeft) {
        // Swap values... our array is one larger than needed so use index LIGHTSAMPLESIZE as hold area
        lightSample[LIGHTSAMPLESIZE].lightLeft = lightSample[i].lightLeft;
        lightSample[i].lightLeft = lightSample[j].lightLeft;
        lightSample[j].lightLeft = lightSample[LIGHTSAMPLESIZE].lightLeft;
      }
      if (lightSample[i].lightCenter > lightSample[j].lightCenter) {
        lightSample[LIGHTSAMPLESIZE].lightCenter = lightSample[i].lightCenter;
        lightSample[i].lightCenter = lightSample[j].lightCenter;
        lightSample[j].lightCenter = lightSample[LIGHTSAMPLESIZE].lightCenter;
      }
      if (lightSample[i].lightRight > lightSample[j].lightRight) {
        // Swap values... our array is one larger than needed so use index LIGHTSAMPLESIZE as hold area
        lightSample[LIGHTSAMPLESIZE].lightRight = lightSample[i].lightRight;
        lightSample[i].lightRight = lightSample[j].lightRight;
        lightSample[j].lightRight = lightSample[LIGHTSAMPLESIZE].lightRight;
      }    
    }
  }
  // Note if want average you could put that in the last position... but don't think we need
  
}

// Helper method to return the current light attributes
LightAttributes LightsClass::getCurrentLightAttributes() {
  setLightAttributes();
  return lightSample[LIGHTSAMPLEVALUE2USE];
}

void LightsClass::showLightAttributes(const int &theAngle) {
  #if USE_LCD 
    sparki.print("Light angle");
    sparki.print(theAngle);
    sparki.print(",lft,");
    sparki.print(lightSample[LIGHTSAMPLEVALUE2USE].lightLeft);
    sparki.print(",cntr,");
    sparki.print(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter);
    sparki.print(",rt,");
    sparki.println(lightSample[LIGHTSAMPLEVALUE2USE].lightRight);
    sparki.updateLCD();
  #else
    Serial.print("LA,<,");
    Serial.print(theAngle);
    Serial.print(",l,");
    Serial.print(lightSample[LIGHTSAMPLEVALUE2USE].lightLeft);
    Serial.print(",c,");
    Serial.print(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter);
    Serial.print(",r,");
    Serial.println(lightSample[LIGHTSAMPLEVALUE2USE].lightRight);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif  
}

void LightsClass::sampleWorldLights() {
  movementsObj.turnToZero();
  for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
    setLightAttributes();

    lightCalibration[i].lightLeft = lightSample[LIGHTSAMPLEVALUE2USE].lightLeft;
    lightCalibration[i].lightCenter = lightSample[LIGHTSAMPLEVALUE2USE].lightCenter;
    lightCalibration[i].lightRight = lightSample[LIGHTSAMPLEVALUE2USE].lightRight;
    lightCalibration[i].flag1 = false;
    lightCalibration[i].flag2 = false;
    showLightAttributes(i*LIGHTSAMPLEANGLE);
 
    movementsObj.turnRight(LIGHTSAMPLEANGLE);
    delay(DELAY_AFTER_MOVEMENT);
  }
}

// Return the delta pct between current value and calibration one, note it can be negative
int LightsClass::getDeltaPct(const int &currentValue, const int &calibrationValue) {
  int deltaAmt = currentValue - calibrationValue; // Delta
  if (deltaAmt > calibrationValue) {
    // Delta is greater than base (meaning more than twice it's original value), return 100 as %
    return 100;
  }
  else
    return (deltaAmt * 100)/calibrationValue;
}

void LightsClass::calculateLightDeltas() {
  int deltaAmt, holdAngle;
  holdAngle = localizationObj.getCurrentAngle();
  movementsObj.turnToZero();
  for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
    setLightAttributes();

    deltaAmt = getDeltaPct(lightSample[LIGHTSAMPLEVALUE2USE].lightLeft, lightCalibration[i].lightLeft);
    lightDeltaPcts[i].leftPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].leftSignBit = (deltaAmt < 0 ? false : true); // Positive values have true

    deltaAmt = getDeltaPct(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter, lightCalibration[i].lightCenter);
    lightDeltaPcts[i].centerPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].centerSignBit = (deltaAmt < 0 ? false : true); // Positive values have true
    #if DEBUGLIGHTS
      Serial.print("LCD,<,");
      Serial.print(i*LIGHTSAMPLEANGLE);
      Serial.print(",");
      Serial.println(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter);
      delay(DELAY_FOR_SERIAL_COMM);
    #endif
    
    deltaAmt = getDeltaPct(lightSample[LIGHTSAMPLEVALUE2USE].lightRight, lightCalibration[i].lightRight);
    lightDeltaPcts[i].rightPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].rightSignBit  = (deltaAmt < 0 ? false : true); // Positive values have true
  
    movementsObj.turnRight(LIGHTSAMPLEANGLE);
    delay(DELAY_AFTER_MOVEMENT);
  }
  movementsObj.turnToAngle(holdAngle);
}

// Little help routine, just returns an integer representing the delta pct (with the correct sign)
int LightsClass::deltaPctHelper(const int &deltaPct, const bool &isPositive) {
  return (isPositive ? deltaPct : -deltaPct);
}

// Helper method
bool LightsClass::numberBetweenRange(const int &theNum, const int &lowValue, const int &highValue) {
  if (lowValue > highValue) {   // It wraps around i.e 11 -> 1
     return ( ( (theNum >= lowValue) || (theNum <= highValue) ) ? true : false);
  }
  else {
    return ( ( (theNum >= lowValue) && (theNum <= highValue) ) ? true : false);
  }
}

// Return angle with highest brightness (or lowest) depending on arg; it will also a particular
// angle and some delta around it (CONSTANT)
// The multFactor passed in should be 1 for highest and -1 for lowest delta, we also have args to ignore
// a range of angles... idea to ignore range that has a light we're not interested in
int LightsClass::getAngleWithBiggestLightDelta(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
  // We'll use the center light for now... may want to change but since we're rotating that should
  // be brightest when pointing directly at light
  if (angle2IgnoreStart >= 0) {
    #if DEBUGLIGHTDELTA
      Serial.println("");
      Serial.print("LBIR,<,");
      Serial.print(angle2IgnoreStart);
      Serial.print(",to,");
      Serial.println(angle2IgnoreEnd);
      delay(DELAY_FOR_SERIAL_COMM);
    #endif
  }
 
  int indexPos  = -1;
  int holdValue = -100; 

  for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
    // Make sure it's not in the range to ignore
    if (numberBetweenRange(i * LIGHTSAMPLEANGLE, angle2IgnoreStart, angle2IgnoreEnd) == false) {
      if ((indexPos == -1) || 
          ((deltaPctHelper(lightDeltaPcts[i].centerPct, lightDeltaPcts[i].centerSignBit) * multFactor) > holdValue)) {
        holdValue = deltaPctHelper(lightDeltaPcts[i].centerPct, lightDeltaPcts[i].centerSignBit) * multFactor;
        indexPos = i;
      }
    }
    else {
      #if DEBUGLIGHTS
        Serial.print("LBI,<,");
        Serial.println(i*LIGHTSAMPLEANGLE);
        delay(DELAY_FOR_SERIAL_COMM);
      #endif
    }
  }
  return indexPos*LIGHTSAMPLEANGLE;
}

// Helper method, will return highest delta, if don't want to ignore angle range pass in -1 and -1 
int LightsClass::getAngleWithHighestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
  return getAngleWithBiggestLightDelta(1, angle2IgnoreStart, angle2IgnoreEnd);
}

// Helper method, will return smallest delta (good if want to find shadows)
int LightsClass::getAngleWithLowestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
  return getAngleWithBiggestLightDelta(-1, angle2IgnoreStart, angle2IgnoreEnd);
}

void LightsClass::showLightDirection(const int &theAngle) {
  int indexOfDelta = theAngle/LIGHTSAMPLEANGLE;
  #if USE_LCD 
    sparki.print("Bright angle");
    sparki.print(theAngle);
    sparki.print(",lft,");
    sparki.print(lightDeltaPcst[indexOfDelta].leftPct);
    sparki.print(",cntr,");
    sparki.print(lightDeltaPcts[indexOfDelta].centerPct);
    sparki.print(",rt,");
    sparki.println(lightDeltaPcts[indexOfDelta].rightPct);
    sparki.updateLCD();
  #else
    Serial.print("LB,<,");
    Serial.print(theAngle);
    Serial.print(",l,");
    Serial.print(deltaPctHelper(lightDeltaPcts[indexOfDelta].leftPct,  lightDeltaPcts[indexOfDelta].leftSignBit));
    Serial.print(",c,");
    Serial.print(deltaPctHelper(lightDeltaPcts[indexOfDelta].centerPct,  lightDeltaPcts[indexOfDelta].centerSignBit));
    Serial.print(",r,");
    Serial.println(deltaPctHelper(lightDeltaPcts[indexOfDelta].rightPct,  lightDeltaPcts[indexOfDelta].rightSignBit));
    delay(DELAY_FOR_SERIAL_COMM);
    
    #if DEBUGLIGHTS
      for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
        int quadrantOfAngle = localizationObj.getQuadrantAngleIsIn(i*LIGHTSAMPLEANGLE);
        Serial.print("LBD,<,");
        Serial.print(i*LIGHTSAMPLEANGLE);
        Serial.print(",l,");
        Serial.print(deltaPctHelper(lightDeltaPcts[i].leftPct,  lightDeltaPcts[i].leftSignBit));
        Serial.print(",c,");
        Serial.print(deltaPctHelper(lightDeltaPcts[i].centerPct, lightDeltaPcts[i].centerSignBit));
        Serial.print(",r,");
        Serial.println(deltaPctHelper(lightDeltaPcts[i].rightPct, lightDeltaPcts[i].rightSignBit));
        delay(DELAY_FOR_SERIAL_COMM);
      }     
    #endif
  #endif  
};

int LightsClass::getAngleWithHighestReading(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  int holdAngle = localizationObj.getCurrentAngle();  // Save current angle
  int saveAngle = holdAngle;
  int tempAngle   = 0;
  int saveReading = 0;
  int nextAngleIncrement = -angleIncrement;  // We don't want to turn on first one so this will make it zero on first pass thru
  for (int i = 0; i < 360; i+=angleIncrement) {
    nextAngleIncrement += angleIncrement;
    tempAngle = (holdAngle + i) % 360;
    if (numberBetweenRange(tempAngle, angle2IgnoreStart, angle2IgnoreEnd) == false) {
      movementsObj.turnRight(nextAngleIncrement);
      nextAngleIncrement = 0; 
      delay(DELAY_AFTER_MOVEMENT);
      
      setLightAttributes();
      if ((lightSample[LIGHTSAMPLEVALUE2USE].lightCenter * multFactor) > saveReading) {
        saveReading = (lightSample[LIGHTSAMPLEVALUE2USE].lightCenter * multFactor);
        saveAngle = tempAngle;
      }
    }
  } 
  movementsObj.turnToAngle(holdAngle); // Point back to the start
  return saveAngle;  // Note if the angle to ignore is everything then we'll return the angle we were oriented at
}

int LightsClass::getAngleWithDimmestLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  return getAngleWithHighestReading(-1, angle2IgnoreStart, angle2IgnoreEnd, angleIncrement);
}
int LightsClass::getAngleWithBrightestLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  return getAngleWithHighestReading(1, angle2IgnoreStart, angle2IgnoreEnd, angleIncrement);
}

void LightsClass::setPotentialLightTargets() {
  // This routine has logic to calculate potential locations of the lights... the lights need to be a distance apart (if
  // we're searching for more thane one).  The array lights2Review has the following when this is done
  //    lights2Review[0] has x,y coordinate of starting
  //    lights2Review[1] has the x,y coordinate of the brightest light (calc'd via triangulation)
  //    lights2Review[2] also has the x,y coordinate starting with
  //    lights2Review[3] has the position of the next brightest light (calc'd via triangulation)
  // Logic
  //    Save current position in pos 0, 2 of lights2Review
  //    Calculate the light deltas
  //      lights2Review[0].angle <- angle to the brightest light
  //      lights2Review[2].angle <- angle of brightest light but ignores angle that first light is at, and angles between LIGHTANGLERANGETOIGNORE
  //    Get the midpoint between angles[0] and [2] (we want to split it and move toward that direction if we can)
  //      we store the angle between them in [1].angle
  //      we store alternate angle at [3].angle (alternate is 180 away from [1].angle)
  //    We want to find angle that we can move toward ([1].angle is preferred)
  //      Get distance available at angle [1].angle
  //      if it's less than minimum triangulation distance then try [3].angle
  //      Use best angle we can
  //    Move forward at that angle a MINTRIANGULATION distance
  //    Calculate light deltas
  //       lights2Review[1].angle <- angle brightest light, it ingores lights2Review[2].angle
  //       lights2Review[3].angle <- angle of brightest light, it ignores lights2Review[0].angle
  //    Triangulate positions [0] with [1] and [2] with [3]
  //
  
  Pose lights2Review[4];  // 0,1 is for first light, 2,3 is for second  
  lights2Review[0] = localizationObj.getPose();
  lights2Review[2] = localizationObj.getPose();
  int angleMoved;

  // Test this it aint working as expected
  // Calculate the light deltas from the 
  localizationObj.writeMsg2Serial("CalcDelta");
  calculateLightDeltas();
  lights2Review[0].angle = getAngleWithHighestLightDelta(-1,-1);  // Use invalid angle to not ignore
  showLightDirection(lights2Review[0].angle);

  if (LIGHTSINWORKSPACE > 1) {
    // Get the next brightest, pass in the angle from the first one... we want to ignore that
    lights2Review[2].angle = getAngleWithHighestLightDelta(localizationObj.getAngle(lights2Review[0].angle-LIGHTANGLERANGETOIGNORE),
                                                           localizationObj.getAngle(lights2Review[0].angle+LIGHTANGLERANGETOIGNORE));
    showLightDirection(lights2Review[2].angle);
  }
  else {
    // We'll use 180 apart
    lights2Review[2].angle = localizationObj.getAngle(lights2Review[1].angle + 180.0);
  }
  // We are going to try and triangulate the position of the lights
  // I'm reusing positions 1 and 3 as work area's... these values won't persist
  // Calculate the angle between the two angles we have, we'll move there to try and triangulate.
  lights2Review[1].angle = localizationObj.getMidpointBetweenTwoAngles(lights2Review[0].angle, lights2Review[2].angle);
  lights2Review[3].angle = localizationObj.getAngle(lights2Review[1].angle + 180); // and calculate 180' away, may need
  
  lights2Review[1].xPos = movementsObj.getDistanceAtAngle(lights2Review[1].angle);
  angleMoved = lights2Review[1].angle; // Save this, want it for the triangulation routine
  if (lights2Review[1].xPos < LIGHTMINTRIANGULATION) {
    // It's less than what we want to triangulate, try 180' away
    lights2Review[3].xPos = movementsObj.getDistanceAtAngle(lights2Review[3].angle);
    if (lights2Review[3].xPos < lights2Review[1].xPos) {
      // Go back to original, it's less than we want but more than at 180'
      movementsObj.turnToAngle(lights2Review[1].angle);
    }
    else
      angleMoved = lights2Review[3].angle;  // We used alternate angle so replace the angleMoved value
  }

  localizationObj.writeMsg2Serial("Triangulating");
  Serial.print(angleMoved);
  localizationObj.writeMsg2Serial(" am");

  // Move forward, we'll try and move the optimal triangulation distance
  while (movementsObj.moveForward(LIGHTOPTIMALTRIANGULATION, ULTRASONIC_MIN_SAFE_DISTANCE, true));

  // Calculate the delta's at a distance
  localizationObj.writeMsg2Serial("CalcNewDelta");
  calculateLightDeltas();
  lights2Review[1].angle = getAngleWithHighestLightDelta(localizationObj.getAngle(lights2Review[2].angle-LIGHTANGLERANGETOIGNORE),
                                                         localizationObj.getAngle(lights2Review[2].angle+LIGHTANGLERANGETOIGNORE));
    
  showLightDirection(lights2Review[1].angle);

  if (LIGHTSINWORKSPACE > 1) {
    // Get the next brightest, pass in the angle from the first one... we want to ignore that
    lights2Review[3].angle = getAngleWithHighestLightDelta(localizationObj.getAngle(lights2Review[0].angle-LIGHTANGLERANGETOIGNORE),
                                                           localizationObj.getAngle(lights2Review[0].angle+LIGHTANGLERANGETOIGNORE));
    showLightDirection(lights2Review[3].angle);
  }

  // Calculate the triangulation
  Pose lightPose;
  if (localizationObj.setPointOfIntersection(lights2Review[0], lights2Review[1], lightPose) == true) {
    localizationObj.showPose(lightPose);
    movementsObj.moveToPose(lightPose);
  }
  else {
    localizationObj.writeMsg2Serial("^Intrsct1stLght");
  }
  if (localizationObj.setPointOfIntersection(lights2Review[2], lights2Review[3], lightPose) == true) {
    localizationObj.showPose(lightPose);
    movementsObj.moveToPose(lightPose);
  }
  else {
    localizationObj.writeMsg2Serial("^Intrsct1stLght");
  }
 
}
