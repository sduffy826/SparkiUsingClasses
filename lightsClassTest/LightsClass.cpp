#include "lightsClass.h"

// =========================================================================================
//      Constructor(s)
// =========================================================================================
LightsClass::LightsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, MovementsClass &movementsObject) {
  ultrasonicObj = &ultrasonicObject;
  localizationObj = &localizationObject;
  movementsObj  = &movementsObject;
}

// =========================================================================================
//      Private methods
// =========================================================================================

// -----------------------------------------------------------------------------------------
// Gets the light attributes, and populates the lightSample array.  The array is sorted that 
// way we can take the value we want, currently the program will use a constant (LIGHTSAMPLEVALUE2USE); 
// that number is the median (at time of this writing).
void LightsClass::setLightSampleAttributesAtCurrentPose() {
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

// -----------------------------------------------------------------------------------------
// Return angle with highest brightness (or lowest) depending on arg.
// The multFactor passed in should be 1 for highest and -1 for lowest delta, we also have 
// args to ignore a range of angles... idea to ignore range that has a light we're not 
// interested in
// NOTE!!! if you don't want to ignore any angles then pass in -1 and -1 (so if you really want
// to ignore a range like -45 to 45 then make sure you pass in 315 and 45 (you can call 
// localizationObj->getAngle(...) to do that for you)
int LightsClass::getAngleWithBiggestLightDeltaPct(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
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
    if (numberBetweenRange((i * LIGHTSAMPLEANGLE), angle2IgnoreStart, angle2IgnoreEnd) == false) {
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

// -----------------------------------------------------------------------------------------
// This takes current readings in the world and returns the brightest or dimmest light (does 
// not compare to calibration values... it's just the angle with highest/lowest) intensity  
// NOTE: this is private... use the methods below as public interface
int LightsClass::getAngleWithHighestCurrentReading(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  int holdAngle = localizationObj->getCurrentAngle();  // Save current angle
  int saveAngle = holdAngle;
  int tempAngle   = 0;
  int saveReading = 0;
  int nextAngleIncrement = -angleIncrement;  // We don't want to turn on first one so this will make it zero on first pass thru
  for (int i = 0; i < 360; i+=angleIncrement) {
    nextAngleIncrement += angleIncrement;
    tempAngle = (holdAngle + i) % 360;
    if (numberBetweenRange(tempAngle, angle2IgnoreStart, angle2IgnoreEnd) == false) {
      movementsObj->turnRight(nextAngleIncrement);
      nextAngleIncrement = 0; 
      delay(DELAY_AFTER_MOVEMENT);
      
      setLightSampleAttributesAtCurrentPose();
      if ((lightSample[LIGHTSAMPLEVALUE2USE].lightCenter * multFactor) > saveReading) {
        saveReading = (lightSample[LIGHTSAMPLEVALUE2USE].lightCenter * multFactor);
        saveAngle = tempAngle;
      }
    }
  } 
  movementsObj->turnToAngle(holdAngle); // Point back to the start
  return saveAngle;  // Note if the angle to ignore is everything then we'll return the angle we were oriented at
}

// -----------------------------------------------------------------------------------------
// Clears a specific LightDelta variable; this is called from clearLightsDeltaSum
void LightsClass::clearLightDelta(LightDelta &lightDelta) {
  lightDelta.deltaSum = 0;
  lightDelta.deltaCounter = 100;  // We don't have sign so we'll use 100 as base
}

// -----------------------------------------------------------------------------------------
// Sets the LightDelta values based on the delta amount passed in, note the deltaCounter is
// stored as a byte so we use a base as 100 meaning 0 and we decrement that value when we
// are passed in a negative amount and increase when we have a positive value, the 
// getLightDeltaCounter method returns a 'user' friendly value so code should be using that
// They should also use getLightsDeltaSum in case we have some 'internal' representation for
// that (currently we don't but who knows)
void LightsClass::setLightDelta(LightDelta &amt, const int &delta) {
  amt.deltaSum += delta;
  if (delta > 0) {
    amt.deltaCounter += (amt.deltaCounter < 200 ? 1 : 0);
  }
  else if (delta < 0) {
    amt.deltaCounter -= (amt.deltaCounter > 0 ? 1 : 0);
  }
}

// -----------------------------------------------------------------------------------------
// This routine takes two LightAttributes and updates the LightsDeltaSum variable, it's really 
// a wrapper for the method that does the work :)
void LightsClass::setLightsDeltaSum(LightsDeltaSum &amts, const LightAttributes &original, const LightAttributes &current) {
  setLightDelta(amts.leftLight,(current.lightLeft - original.lightLeft));
  setLightDelta(amts.centerLight,(current.lightCenter - original.lightCenter));
  setLightDelta(amts.rightLight,(current.lightRight - original.lightRight));
}

// -----------------------------------------------------------------------------------------
// Helper method to compare two LightDeltas and return the id associated with the record that's 
// most significant, the last argument is a multiplication factor... a -1 value means you want 
// the light that decreased more; note you pass the id you want to use to signify the lights; see 
int LightsClass::lightDeltaAmountHelper(const LightDelta &lightDelta1, const LightDelta &llightDelta2, const byte &lightId1, const byte &lightId2, const int &multFactor) {
  // Make sure that our delta counter is above the threshold we want
  if ( ((getLightDeltaCounter(lightDelta1) * multFactor) >= LIGHTDELTAS2ACTON) || ((getLightDeltaCounter(lightDelta2) * multFactor) >= LIGHTDELTAS2ACTON) ) {
    if (getLightDeltaCounter(lightDelta1) == getLightDeltaCounter(lightDelta2)) {
      // Both lights have increased or decreased the same amount of times, use the one with most signifcant light
      return ( (getLightsDeltaSum(light1)*multFactor) > (getLightsDeltaSum(light2)*multFactor) ? lightId1 : lightId2);
    }
    else {
      // Return the one that has increased (or decreased) more frequently
      return ( (getLightDeltaCounter(lightDelta1) * multFactor) > (getLightDeltaCounter(lightDelta2) * multFactor) ? lightId1 : lightId2 );
    }
  }
  else 
    return -1;
}
// =========================================================================================
//      Public methods
// =========================================================================================

// -----------------------------------------------------------------------------------------
// This takes a sample of the 'world' lights, the values are stored in the lightCalibration 
// array for the respective angles
void LightsClass::sampleWorldLights() {
  movementsObj->turnToZero();
  for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
    setLightSampleAttributesAtCurrentPose();

    lightCalibration[i].lightLeft = lightSample[LIGHTSAMPLEVALUE2USE].lightLeft;
    lightCalibration[i].lightCenter = lightSample[LIGHTSAMPLEVALUE2USE].lightCenter;
    lightCalibration[i].lightRight = lightSample[LIGHTSAMPLEVALUE2USE].lightRight;
    lightCalibration[i].flag1 = false;
    lightCalibration[i].flag2 = false;
    showSampledLightAttributes(i*LIGHTSAMPLEANGLE);
 
    movementsObj->turnRight(LIGHTSAMPLEANGLE);
    delay(DELAY_AFTER_MOVEMENT);
  }
}

// -----------------------------------------------------------------------------------------
// Calculate the light deltas... this takes samples of 360' from the position you are at and
// compares it with the calibration values... it updates the lightDeltaPcts array with the 
//net change between them
void LightsClass::calculateWorldLightDeltas() {
  int deltaAmt, holdAngle;
  holdAngle = localizationObj->getCurrentAngle();
  movementsObj->turnToZero();
  for (int i = 0; i < LIGHTCALIBRATIONARRAYSIZE; i++) {
    setLightSampleAttributesAtCurrentPose();

    deltaAmt = getLightDeltaPctBetween2Values(lightSample[LIGHTSAMPLEVALUE2USE].lightLeft, lightCalibration[i].lightLeft);
    lightDeltaPcts[i].leftPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].leftSignBit = (deltaAmt < 0 ? false : true); // Positive values have true

    deltaAmt = getLightDeltaPctBetween2Values(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter, lightCalibration[i].lightCenter);
    lightDeltaPcts[i].centerPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].centerSignBit = (deltaAmt < 0 ? false : true); // Positive values have true
    #if DEBUGLIGHTS
      Serial.print("LCD,<,");
      Serial.print(i*LIGHTSAMPLEANGLE);
      Serial.print(",");
      Serial.println(lightSample[LIGHTSAMPLEVALUE2USE].lightCenter);
      delay(DELAY_FOR_SERIAL_COMM);
    #endif
    
    deltaAmt = getLightDeltaPctBetween2Values(lightSample[LIGHTSAMPLEVALUE2USE].lightRight, lightCalibration[i].lightRight);
    lightDeltaPcts[i].rightPct = (deltaAmt < 0 ? -deltaAmt : deltaAmt);
    lightDeltaPcts[i].rightSignBit  = (deltaAmt < 0 ? false : true); // Positive values have true
  
    movementsObj->turnRight(LIGHTSAMPLEANGLE);
    delay(DELAY_AFTER_MOVEMENT);
  }
  movementsObj->turnToAngle(holdAngle);
}

// -----------------------------------------------------------------------------------------
//  Method to return the current light attributes, this is public method
LightAttributes LightsClass::getLightAttributesAtCurrentPose() {
  setLightSampleAttributesAtCurrentPose();
  return lightSample[LIGHTSAMPLEVALUE2USE];
}

// -----------------------------------------------------------------------------------------
// Routine to clear the variable that holds the LightsDeltaSum... you call this to reset the delta counters and sums (maybe after you took some action)
void LightsClass::clearLightsDeltaSum(LightsDeltaSum &lightsDeltaSum) {
  clearLightDelta(lightsDeltaSum.leftLight);
  clearLightDelta(lightsDeltaSum.centerLight);
  clearLightDelta(lightsDeltaSum.rightLight);
}

// -----------------------------------------------------------------------------------------
// Helper method to get the total light delta amount
int LightsClass::getLightsDeltaSum(const LightDelta &lightDelta) {
  return lightDelta.deltaSum;
}

// -----------------------------------------------------------------------------------------
// Helper to get the frequency that the light has changed (- means it's decreased that many times)
int LightsClass::getLightDeltaCounter(const LightDelta &lightDelta) {
  return 100-lightDeltaCounter;
}

// -----------------------------------------------------------------------------------------
// This routine calls method 'setLightsDeltaSum' to set the LightsDeltaSum variable, it keeps track of the delta amounts between two
// LightAttributes which are the other args passed in.
// It then calls method 'lightDeltaAmountsHelper' to determine which light source is the most significant... it does this by 
// seeing which one has changed more frequently (not the light change value, it's frequency).  If two lights have the same 
// frequency of change then it'll return the one with the most significan change.
int LightsClass::getLightAngleToTurnToBasedOnDeltaSum(LightsDeltaSum &amts, const LightAttributes &original, const LightAttributes &current) {
  setLightsDeltaSum(amts, original, current);
  // Get id of lights that are more significant (the last parm (1) means want increasing values... if want decreasing then pass -1)
  int idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightLeft, amts.leftIncCnt, 1, amts.lightCenter, amts.centerIncCnt, 2, 1);
  if (idOfLight2Point2 = 1) { // Left is significant, compare it to the right light
    idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightLeft, amts.leftIncCnt, 1, amts.lightRight, amts.rightIncCnt, 3, 1);
  }
  else { // Center is more significant of the left light, compare it to the right.
    idOfLight2Point2 = lightDeltaAmountsHelper(amts.lightCenter, amts.centerIncCnt, 2,amts.lightRight, amts.rightIncCnt, 3, 1);
  }
  if (idOfLight2Point2 > 0) {
    // One is signifcant
    return ((idOfLight2Point2 - 2) * LIGHTDELTAANGLE2TURN);  // so this will return -LIGHTDELTAANGLE2TURN, 0 or LIGHTDELTAANGLETOTURN (for left, center, right)
  }
  return 0;
}

// -----------------------------------------------------------------------------------------
// Helper method, will return highest delta, if don't want to ignore angle range pass in -1 and -1 
int LightsClass::getAngleWithHighestLightDeltaPct(const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
  return getAngleWithBiggestLightDeltaPct(1, angle2IgnoreStart, angle2IgnoreEnd);
}

// -----------------------------------------------------------------------------------------
// Helper method, will return smallest delta (good if want to find shadows)
int LightsClass::getAngleWithLowestLightDeltaPct(const int &angle2IgnoreStart, const int &angle2IgnoreEnd) {
  return getAngleWithBiggestLightDeltaPct(-1, angle2IgnoreStart, angle2IgnoreEnd);
}

// -----------------------------------------------------------------------------------------
// Get the angle that has the highest current light (ignores calibation)
int LightsClass::getAngleWithBrightestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  return getAngleWithHighestCurrentReading(1, angle2IgnoreStart, angle2IgnoreEnd, angleIncrement);
}

// -----------------------------------------------------------------------------------------
// Get the angle with the dimmest current light
int LightsClass::getAngleWithDimmestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement) {
  return getAngleWithHighestCurrentReading(-1, angle2IgnoreStart, angle2IgnoreEnd, angleIncrement);
}

// -----------------------------------------------------------------------------------------
// Utility method, return the delta pct between current value and calibration one, note it can be negative
int LightsClass::getLightDeltaPctBetween2Values(const int &currentValue, const int &calibrationValue) {
  int deltaAmt = currentValue - calibrationValue; // Delta
  if (deltaAmt > calibrationValue) {
    // Delta is greater than base (meaning more than twice it's original value), return 100 as %
    return 100;
  }
  else
    return (deltaAmt * 100)/calibrationValue;
}

// -----------------------------------------------------------------------------------------
// Little helper routine, just returns an integer representing the delta pct (with the correct sign)
int LightsClass::deltaPctHelper(const int &deltaPct, const bool &isPositive) {
  return (isPositive ? deltaPct : -deltaPct);
}

// -----------------------------------------------------------------------------------------
// Helper method, checks that a number is between a range
bool LightsClass::numberBetweenRange(const int &theNum, const int &lowValue, const int &highValue) {
  if (lowValue > highValue) {   // It wraps around i.e 11 -> 1
     return ( ( (theNum >= lowValue) || (theNum <= highValue) ) ? true : false);
  }
  else {
    return ( ( (theNum >= lowValue) && (theNum <= highValue) ) ? true : false);
  }
}

// -----------------------------------------------------------------------------------------
// Routine to triangulate light sources

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
  lights2Review[0] = localizationObj->getPose();
  lights2Review[2] = localizationObj->getPose();
  int angleMoved;

  // Test this it aint working as expected
  // Calculate the light deltas from the 
  localizationObj->writeMsg2Serial("CalcDelta");
  calculateWorldLightDeltas();
  lights2Review[0].angle = getAngleWithHighestLightDeltaPct(-1,-1);  // Use invalid angle to not ignore
  showLightDeltaPctForAngle(lights2Review[0].angle);

  if (LIGHTSINWORKSPACE > 1) {
    // Get the next brightest, pass in the angle from the first one... we want to ignore that
    lights2Review[2].angle = getAngleWithHighestLightDeltaPct(localizationObj->getAngle(lights2Review[0].angle-LIGHTANGLERANGETOIGNORE),
                                                              localizationObj->getAngle(lights2Review[0].angle+LIGHTANGLERANGETOIGNORE));
    showLightDeltaPctForAngle(lights2Review[2].angle);
  }
  else {
    // We'll use 180 apart
    lights2Review[2].angle = localizationObj->getAngle(lights2Review[1].angle + 180.0);
  }
  // We are going to try and triangulate the position of the lights
  // I'm reusing positions 1 and 3 as work area's... these values won't persist
  // Calculate the angle between the two angles we have, we'll move there to try and triangulate.
  lights2Review[1].angle = localizationObj->getMidpointBetweenTwoAngles(lights2Review[0].angle, lights2Review[2].angle);
  lights2Review[3].angle = localizationObj->getAngle(lights2Review[1].angle + 180); // and calculate 180' away, may need
  
  lights2Review[1].xPos = movementsObj->getDistanceAtAngle(lights2Review[1].angle);
  angleMoved = lights2Review[1].angle; // Save this, want it for the triangulation routine
  if (lights2Review[1].xPos < LIGHTMINTRIANGULATION) {
    // It's less than what we want to triangulate, try 180' away
    lights2Review[3].xPos = movementsObj->getDistanceAtAngle(lights2Review[3].angle);
    if (lights2Review[3].xPos < lights2Review[1].xPos) {
      // Go back to original, it's less than we want but more than at 180'
      movementsObj->turnToAngle(lights2Review[1].angle);
    }
    else
      angleMoved = lights2Review[3].angle;  // We used alternate angle so replace the angleMoved value
  }

  localizationObj->writeMsg2Serial("Triangulating");
  Serial.print(angleMoved);
  localizationObj->writeMsg2Serial(" am");

  // Move forward, we'll try and move the optimal triangulation distance
  while (movementsObj->moveForward(LIGHTOPTIMALTRIANGULATION, ULTRASONIC_MIN_SAFE_DISTANCE, true));

  // Calculate the delta's at a distance
  localizationObj->writeMsg2Serial("CalcNewDelta");
  calculateWorldLightDeltas();
  lights2Review[1].angle = getAngleWithHighestLightDeltaPct(localizationObj->getAngle(lights2Review[2].angle-LIGHTANGLERANGETOIGNORE),
                                                            localizationObj->getAngle(lights2Review[2].angle+LIGHTANGLERANGETOIGNORE));
    
  showLightDeltaPctForAngle(lights2Review[1].angle);

  if (LIGHTSINWORKSPACE > 1) {
    // Get the next brightest, pass in the angle from the first one... we want to ignore that
    lights2Review[3].angle = getAngleWithHighestLightDeltaPct(localizationObj->getAngle(lights2Review[0].angle-LIGHTANGLERANGETOIGNORE),
                                                              localizationObj->getAngle(lights2Review[0].angle+LIGHTANGLERANGETOIGNORE));
    showLightDeltaPctForAngle(lights2Review[3].angle);
  }

  // Calculate the triangulation
  Pose lightPose;
  if (localizationObj->setPointOfIntersection(lights2Review[0], lights2Review[1], lightPose) == true) {
    localizationObj->showPose(lightPose);
    movementsObj->moveToPose(lightPose);
  }
  else {
    localizationObj->writeMsg2Serial("^Intrsct1stLght");
  }
  if (localizationObj->setPointOfIntersection(lights2Review[2], lights2Review[3], lightPose) == true) {
    localizationObj->showPose(lightPose);
    movementsObj->moveToPose(lightPose);
  }
  else {
    localizationObj->writeMsg2Serial("^Intrsct1stLght");
  }
}

// ==========================================================================================
// Methods predominantly for debugging... they show values 
// ==========================================================================================

// -----------------------------------------------------------------------------------------
// Helper method to show light attributes, since I don't know the angle the light attributes 
// where taken you should pass that in
void LightsClass::showLightAttributes(char *msgStr, const LightAttributes &liteAttr, const int &theAngle) {
  #if USE_LCD 
    sparki.print("LghtAttr,");
    sparki.print(msgStr);
    sparki.print(",<,");
    sparki.print(theAngle);
    sparki.print(",lft,");
    sparki.print(liteAttr.lightLeft);
    sparki.print(",cntr,");
    sparki.print(liteAttr.lightCenter);
    sparki.print(",rt,");
    sparki.println(liteAttr.lightRight);
    sparki.updateLCD();
  #else
    Serial.print("LA,");
    sparki.print(msgStr);
    sparki.print(",<,");
    Serial.print(theAngle);
    Serial.print(",l,");
    Serial.print(liteAttr.lightLeft);
    Serial.print(",c,");
    Serial.print(liteAttr.lightCenter);
    Serial.print(",r,");
    Serial.println(liteAttr.lightRight);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif  
}

// -----------------------------------------------------------------------------------------
// Helper to show the light values in the 'lightSample' array; we use the LIGHTSAMPLEVALUE2USE to specify which one
void LightsClass::showSampledLightAttributes(const int &theAngle) {
  showLightAttributes("Sam",lightSample[LIGHTSAMPLEVALUE2USE],theAngle);
}

// -----------------------------------------------------------------------------------------
// Little helper to show the 'base' calibration light for an angle
void LightsClass::showCalibrationLightAtAngle(const int &theAngle) {
  showLightAttributes("Cal",lightCalibration[theAngle/LIGHTSAMPLEANGLE], theAngle);
}

// -----------------------------------------------------------------------------------------
// Little helper to show what light values are for a given 'world' angle; it'll show
void LightsClass::showLightDeltaPctForAngle(const int &theAngle) {
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
        int quadrantOfAngle = localizationObj->getQuadrantAngleIsIn(i*LIGHTSAMPLEANGLE);
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

// Helper to show the light deltas (these are not percentage of change), the first number in the set is the total delta for that light, the second
// number is the count of increases we've seen (a + number); if that is negative it represents decreases in light... note you could have a sum amount
// that's + and a delta count that's - (that'd occur if you had a big jump or decline in light values)
void LightsClass::showLightsDeltaSum(const LightsDeltaSum &amts) {
  Serial.print("LDA,l");
  Serial.print(getLightsDeltaSum(amts.leftLight));
  Serial.print(",/");
  Serial.print(getLightDeltaCounter(amts.leftLight));
  Serial.print(",c,");
  Serial.print(getLightsDeltaSum(amts.centerLight));
  Serial.print(",/");
  Serial.print(getLightDeltaCounter(amts.centerLight));
  Serial.print(",r,");
  Serial.print(getLightsDeltaSum(amts.rightLight));
  Serial.print(",/");
  Serial.println(getLightDeltaCounter(amts.rightLight));
}
