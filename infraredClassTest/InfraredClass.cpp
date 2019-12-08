#include "infraredClass.h"
      
// =========================================================================================
//      Constructor(s)
// =========================================================================================
InfraredClass::InfraredClass(LocalizationClass &localizationObject, MovementsClass &movementsObject) {
  localizationObj = &localizationObject;
  movementsObj    = &movementsObject;
  lineWidthInMM   = 0;
  clearInfraredAttributes(infraredBase);

  unsigned long startTime = millis();
  sparki.moveRight(90);
  millisFor90Degrees = millis() - startTime;
  sparki.moveLeft(90);
}

// =========================================================================================
//      Methods 
// -----------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Adjust the angle from the currentPose to the targetPose and return the distance
// required to get there, note this does not account for obstacles between current
// pose and the target (if u wanted to then return min of distance2Move and openSpace
// NOTE: REVIEW this you may want to move this into the movementsClass.. but for now
// not sure if other changes will be required so left here
float InfraredClass::adjustAngleToPose(const Pose &targetPose) {
  
  Pose currentPose = localizationObj->getPose();
  
  if (DEBUGINFRARED) {
    Serial.print("(aa2p)cur");
    localizationObj->showPose(currentPose);
    Serial.print("(aa2p)tar");
    localizationObj->showPose(targetPose);
  }
 
  int targetAngle = localizationObj->calculateAngleBetweenPoints(currentPose.xPos, currentPose.yPos, targetPose.xPos, targetPose.yPos);
  
  // Turn to specific angle and return distance in front of you :)
  float openSpace = movementsObj->getDistanceAtAngle(targetAngle);
  float distance2Move = localizationObj->distanceBetweenPoses(currentPose, targetPose);

  if (DEBUGINFRARED) {
    Serial.print("<:");
    Serial.print(targetAngle);
    Serial.print(" dist: ");
    Serial.println(distance2Move);
  }
  
  return distance2Move;
}

// -----------------------------------------------------------------------------------------
// We've drifted, try to straighten back out
void InfraredClass::adjustForDrifting(const bool &driftingLeft) {
  if (driftingLeft) {
    //if (DEBUGINFRARED) localizationObj->writeMsg2Serial("IA,AdjustLeftDrift");
    localizationObj->writeMsg2Serial("IR,LeftDrift");
    movementsObj->turnLeft(90 - INFRARED_DRIFT_ADJUSTMENT_DEGREES);
    while (movementsObj->moveBackward((getLineWidth()/2.0), ULTRASONIC_MIN_SAFE_DISTANCE, false));
    movementsObj->turnRight(90);
  }
  else {
    //if (DEBUGINFRARED) localizationObj->writeMsg2Serial("IA,AdjustRightDrift");
    localizationObj->writeMsg2Serial("IR,RightDrift");
    movementsObj->turnRight(90 - INFRARED_DRIFT_ADJUSTMENT_DEGREES);
    while (movementsObj->moveBackward((getLineWidth()/2.0), ULTRASONIC_MIN_SAFE_DISTANCE, false));
    movementsObj->turnLeft(90);
  }
}

// Routine below adjusts the position of the robot on the line, idea is to call this when you
// start a new path... it'll cause the robot to straighten itself out... the value returned is
// the next interval that should be checked... if no adjustment was made then the interval
// returned doubles from it's last value.  Note: the robot should not be moving when this is
// called; I expect the caller to handle saving state, stopping and restarting
// Checks the that we're on the line, pass in the lastDistance interval checked
int InfraredClass::adjustPositionOnLine(const int &lastIntervalDistance) {

  int distance2Check = lastIntervalDistance;
  if (DEBUGINFRARED) Serial.println("AdjustPosOnLine (apol)");

  if ((onLine(infraredBase.lineCenter) == true) && (onIntersection(infraredBase.edgeLeft) == false)) {
    int leftAngle  = calcAngleFromEdgeToTape(true, infraredBase.edgeLeft, millisFor90Degrees);
    int rightAngle = calcAngleFromEdgeToTape(false, infraredBase.edgeRight, millisFor90Degrees);
    int deltaAngle = abs(leftAngle - rightAngle);  // Delta between the left and right angles

    if (DEBUGINFRARED) {        
      Serial.print("(apol),l<:");
      Serial.print(leftAngle);
      Serial.print(",r<:");
      Serial.print(rightAngle);
      Serial.print(",delt:");
      Serial.print(deltaAngle);
      Serial.print(",adj:");
      Serial.println((deltaAngle + 1)/2 + 1);
      Serial.flush();
    }
    
    if (deltaAngle > INFRARED_DRIFT_ADJUSTMENT_DEGREES) {
      deltaAngle = (deltaAngle + 1)/2 + 1;  // Move 1 degree more than 1/2 way
      // We want the localization object updated so we'll use movementsObj to adjust angle
      if (leftAngle > rightAngle)
        movementsObj->turnRight(deltaAngle);
      else
        movementsObj->turnLeft(deltaAngle);
      distance2Check = 1;  // Made adjustment go back to 1
    }
    else // No drift increase the distance to check
      distance2Check += distance2Check;
  }
  else { 
    // On an instersection or not on a line
    distance2Check = getLineWidth() + 2;
    if (DEBUGINFRARED) {
      Serial.println("aPOL,NotOnline or am at Intersection");
    }
  }
    
  return distance2Check;
}

void InfraredClass::adjustToLineCenter() {

  byte theFlag = 1;  // Backward mode
  int leftAngle, rightAngle, deltaAngle;
  
  // backup 2cm so that we don't mistakingly read intersection
  while (movementsObj->moveBackward(2.0, ULTRASONIC_MIN_SAFE_DISTANCE, false));

  while (theFlag != 0) {
    leftAngle  = calcAngleFromEdgeToTape(true, infraredBase.edgeLeft, millisFor90Degrees);
    rightAngle = calcAngleFromEdgeToTape(false, infraredBase.edgeRight, millisFor90Degrees);
    deltaAngle = abs(leftAngle - rightAngle);  // Delta between the left and right angles

    // Make adjustment if required or we need to move back to original position
    if ((deltaAngle > INFRARED_DRIFT_ADJUSTMENT_DEGREES) || (theFlag == 2)) {
      deltaAngle = (deltaAngle + 1)/2 + 1;  // Move 1 degree more than 1/2 way
      // We want the localization object updated so we'll use movementsObj to adjust angle
      if (theFlag == 2) {  // Forward
        if (leftAngle > rightAngle)
          movementsObj->turnRight(deltaAngle);
        else
          movementsObj->turnLeft(deltaAngle);
        while (movementsObj->moveForward(4.0, ULTRASONIC_MIN_SAFE_DISTANCE, false));
        if (leftAngle > rightAngle)
          movementsObj->turnLeft(deltaAngle);
        else
          movementsObj->turnRight(deltaAngle);
        theFlag = 1;  // Next mode is reverse
      }
      else {  // Backward
        if (leftAngle > rightAngle)
          movementsObj->turnLeft(deltaAngle);
        else
          movementsObj->turnRight(deltaAngle);
        while (movementsObj->moveBackward(5.0, ULTRASONIC_MIN_SAFE_DISTANCE, false));
        if (leftAngle > rightAngle)
          movementsObj->turnRight(deltaAngle);
        else
          movementsObj->turnLeft(deltaAngle);
        while (movementsObj->moveForward(5.0, ULTRASONIC_MIN_SAFE_DISTANCE, false));
        //theFlag = 2;
      }
    }
    else theFlag = 0;  // No adjustment needed and no repositioning required
  } 
  
  /*
  int distance2Check = lastIntervalDistance;
  Serial.println("AdjustPosOnLine (apol)");

  if ((onLine(infraredBase.lineCenter) == true) && (onIntersection(infraredBase.edgeLeft) == false)) {
    int leftAngle  = calcAngleFromEdgeToTape(true, infraredBase.edgeLeft, millisFor90Degrees);
    int rightAngle = calcAngleFromEdgeToTape(false, infraredBase.edgeRight, millisFor90Degrees);
    int deltaAngle = abs(leftAngle - rightAngle);  // Delta between the left and right angles

    if (DEBUGINFRARED) {        
      Serial.print("(apol),l<:");
      Serial.print(leftAngle);
      Serial.print(",r<:");
      Serial.print(rightAngle);
      Serial.print(",delt:");
      Serial.print(deltaAngle);
      Serial.print(",adj:");
      Serial.println((deltaAngle + 1)/2 + 1);
      Serial.flush();
    }
    
    if (deltaAngle > INFRARED_DRIFT_ADJUSTMENT_DEGREES) {
      deltaAngle = (deltaAngle + 1)/2 + 1;  // Move 1 degree more than 1/2 way
      // We want the localization object updated so we'll use movementsObj to adjust angle
      if (leftAngle > rightAngle)
        movementsObj->turnRight(deltaAngle);
      else
        movementsObj->turnLeft(deltaAngle);
      distance2Check = 1;  // Made adjustment go back to 1
    }
    else // No drift increase the distance to check
      distance2Check += distance2Check;
  }
  else { 
    // On an instersection or not on a line
    distance2Check = getLineWidth() + 2;
    if (DEBUGINFRARED) {
      Serial.println("aPOL,NotOnline or am at Intersection");
    }
  }
    
  return distance2Check;
  */
} 

// Helper routine to sample the edge readings; will return appropriate flags for their
//   state (based on samples taken)
void InfraredClass::adjustToTapeHelper(bool &leftEdge, bool &rightEdge) {
  byte leftOn = 128, rightOn = 128;
  int theLReading, theRReading;
  for (byte idx = 0; idx < INFRARED_TAPE_SAMPLE_SIZE; idx++) {
    theLReading = sparki.edgeLeft();
    theRReading = sparki.edgeRight();
    if (DEBUGINFRARED) {
      Serial.print("aTTH,l,"); Serial.print(theLReading);
      Serial.print(",r,"); Serial.print(theRReading);
    }
    leftOn += (lineFlagHelper(theLReading,infraredBase.edgeLeft) ? 1 : -1);  // add 1 if edge indicator is on
    rightOn += (lineFlagHelper(theRReading,infraredBase.edgeRight) ? 1 : -1); 
    if (DEBUGINFRARED) {
      Serial.print(",leftOn:"); Serial.print(leftOn);
      Serial.print(",rightOn:"); Serial.println(rightOn);
    }
    delay(3);
  }
  leftEdge = (leftOn > 128 ? true : (leftOn < 128 ? false : leftEdge));  // If > 128 it's on, if less it's off, if equal return existing value
  rightEdge = (rightOn > 128 ? true : (rightOn < 128 ? false : rightEdge));  // If > 128 it's on, if less it's off, if equal return existing value
  
  if ((DEBUGINFRARED) && (leftEdge || rightEdge)) {
    Serial.println(" ");
    Serial.print("leftEdge:");
    Serial.print((leftEdge ? "T" : "F"));
    Serial.print(" rightEdge:");
    Serial.println((rightEdge ? "T" :"F"));    
  }
}

// Routine to adjust to the center of the tape... see logic below
bool InfraredClass::adjustToTape() {
  // Logic
  //   turn 90'
  //   backup and log difference between edge sensor time they went on (this is edge of tape)
  //   keep backing up till sensors go off... log delta... the time between them should be same, if not repeat
  //   the delta between time for left/right edges represent the amount of angle adjustment you 
  //     need to make... correct the angle using those values
  //   u know the tape width and how long you've travelled... move forward to middle of tape and turn
  //     back 90'
  unsigned long edgeLeftTime = 0, edgeRightTime = 0;
  float distanceTraveled1 = 0.0, distanceTraveled2 = 0.0, totalDistance = 0.0;
  bool leftIndicator, rightIndicator;
  int startDelta = 0, endDelta = 0;

  // Move backward a little, turn right and move forward a little just to ensure we're not on the tape
  while (movementsObj->moveBackward(1.0,ULTRASONIC_MIN_SAFE_DISTANCE, false)); 
  movementsObj->turnRight(90);
  while (movementsObj->moveForward(1.0,ULTRASONIC_MIN_SAFE_DISTANCE, false)); 
  
  // Stay in loop for up to twice the tape width or we found the distance traveled (used 1.0 as it should never be less than that but coulda used 0.1 too)
  while (movementsObj->moveBackward((2*INFRARED_LINE_WIDTH),ULTRASONIC_MIN_SAFE_DISTANCE, false) && (distanceTraveled2 < 0.01)) {
    adjustToTapeHelper(leftIndicator, rightIndicator);
    if (DEBUGINFRARED) {
      Serial.print("leftIndicator:"); Serial.print(leftIndicator);
      Serial.print(" rightIndicator:"); Serial.println(rightIndicator);
    }
    if (startDelta == 0) { // Trying to find start of tape
      if (leftIndicator == true && edgeLeftTime == 0) edgeLeftTime = millis();
      if (rightIndicator == true && edgeRightTime == 0) edgeRightTime = millis();
      if (edgeLeftTime > 0 && edgeRightTime > 0) {
        distanceTraveled1 = movementsObj->getDistanceTraveledSoFar();
        totalDistance += distanceTraveled1;
        movementsObj->stopMoving();
        startDelta = ((edgeLeftTime - edgeRightTime) != 0) ? (edgeLeftTime - edgeRightTime) : 1; // want startDelta to not be 0        
        edgeLeftTime = edgeRightTime = 0;
        sparki.beep(); // Tell em we found the start of the tape
        if (DEBUGINFRARED) {        
          Serial.print("startDelta != 0, startDelta:"); Serial.print(startDelta);
          Serial.print(" distanceTraveled1:"); Serial.println(distanceTraveled1);
        }
      }
    }
    else {      
      if (leftIndicator == false && edgeLeftTime == 0) edgeLeftTime = millis();
      if (rightIndicator == false && edgeRightTime == 0) edgeRightTime = millis();
      if (edgeLeftTime > 0 && edgeRightTime > 0) {
        distanceTraveled2 = movementsObj->getDistanceTraveledSoFar() + 0.01;
        totalDistance += distanceTraveled2;
        movementsObj->stopMoving();
        sparki.beep();  // Tell em we found the end of the tape
        endDelta = ((edgeLeftTime - edgeRightTime) != 0) ? (edgeLeftTime - edgeRightTime) : 1; // want startDelta to not be 0        
      }
    }
    totalDistance = movementsObj->getDistanceTraveledSoFar(); 
    // delay(2);  // adjustTapeHelper has a delay in it, don't think we need this one
  }
  // If distanceTraveled1 or distanceTraveled2 have been set then getDistanceTraveledSoFar was
  // reset so we need to add those distances back in.
  totalDistance += distanceTraveled1 + distanceTraveled2;
  
  movementsObj->stopMoving();
  if (DEBUGINFRARED) {
    Serial.print("out, endDelta:"); Serial.print(endDelta);
    Serial.print(" totalDistance:"); Serial.print(totalDistance);
    Serial.print(" distanceTraveled2:");  Serial.println(distanceTraveled2);
  }
  // If found tape 
  if (distanceTraveled2 > 0.0) {
    distanceTraveled1 = 0.3;  // The sensor is .3 cm from the tape when it stops so we need to add that back on
    // If 3' or more adjustment required then make it
    if (abs(startDelta) >= (millisFor90Degrees / 30)) {
      // If the delta amounts are close to eachother (within 2' time) (this is done to avoid when we have a
      // wide variation in deltas... which would signify a noisy reading)
      if (abs(startDelta - endDelta) <= (millisFor90Degrees / 45)) {
        endDelta = (startDelta + endDelta) / 4;
        if (endDelta > 0) {
          // If delta is positive it means the left sensor was seen after the right one, turn left
          sparki.moveLeft();
          edgeLeftTime = millis();
          while ((millis()-edgeLeftTime) < endDelta);
          sparki.moveStop();
        }
        else
          if (endDelta < 0) {
            sparki.moveRight();
            edgeLeftTime = millis();
            while ((millis()-edgeLeftTime) < (-endDelta));
            sparki.moveStop();        
          }

       // We add 1/2 the endDelta, the distanceTraveled2 is based on the second sensor
       // triggering so we'll assume after repositioning it's 1/2 delta time back further
       // than what the sensor says
       distanceTraveled1 += (float)(VELOCITY_CM_P_SEC * abs(endDelta)/2) / 1000.0;
          
      } // end checking that delta's are within 1' 
      else 
        if (DEBUGINFRARED) Serial.print("Deltas not close (within 2'))");
    }

    // Now the sensor should be on the tape... move forward so robot center is over the tape... it's
    // distance from sensor to center of bot + 1/2 distanceTraveled2, we'll assign this to distanceTraveled1 :)
    distanceTraveled1 += INFRARED_SENSOR_FORWARD_OF_CENTER + (.5 * distanceTraveled2);
    while (movementsObj->moveForward(distanceTraveled1,ULTRASONIC_MIN_SAFE_DISTANCE, false));  
    leftIndicator = true; // We'll reuse this for return variable, this signifies that we adjusted
  }
  else {    
    // didn't find valid values so move back to start (we subract 1cm for the original movement forward)
    totalDistance -= 1.0;
    while (movementsObj->moveForward(totalDistance,ULTRASONIC_MIN_SAFE_DISTANCE, false));  
    leftIndicator = false;
  }
  movementsObj->turnLeft(90);  // Turn back
  while (movementsObj->moveForward(1.0,ULTRASONIC_MIN_SAFE_DISTANCE, false)); // Move back to original position
  return leftIndicator;  // Not the left indicator, it's reused to signify if we adjusted angle :)
}

// -----------------------------------------------------------------------------------------
// Assigning structs is just reference assignment, this does the byname... didn't overload assignment
// as not sure if arduino c++ handles it and this was easy
/* Commented out... we don't need to do this... originally thought assignment would be a reference assignment
 *  but struct assigments are a memberwise value assignment
void InfraredClass::assignSourceAttributesToTarget(const InfraredAttributes &source, InfraredAttributes &target) {
  target.edgeLeft   = source.edgeLeft;
  target.lineLeft   = source.lineLeft;
  target.lineCenter = source.lineCenter;
  target.lineRight  = source.lineRight;
  target.edgeRight  = source.edgeRight;

  // Line flags
  target.el_line = source.el_line;
  target.ll_line = source.ll_line;
  target.lc_line = source.lc_line;
  target.lr_line = source.lr_line;
  target.er_line = source.er_line;

  // State flags
  target.driftLeft      = source.driftLeft;
  target.driftRight     = source.driftRight;
  target.onLine         = source.onLine;
  target.atExit         = source.atExit;
  target.startLeftPath  = source.startLeftPath;
  target.startRightPath = source.startRightPath;
  target.endLeftPath    = source.endLeftPath;
  target.endRightPath   = source.endRightPath;
  target.atEntrance     = source.atEntrance;  
}
*/
// -------------------------------------------------------------------------------------------------
// Calculate the angle from the edge reading to the tape, if firstArg is true then you want to calculate
// the left edge otherwise it'll do the right one, the other args are the 'base' reading and the number
// of milliseconds needed to turn 90 degrees (I use that to calculate the angle)       
// NOTE: The sparki is back at it's original position after this routine so it doesn't affect 
//       localization.
int InfraredClass::calcAngleFromEdgeToTape(const bool &leftEdge, const int &baseReading, const int &millisFor90Degrees, const bool &isRecall=false) {
  unsigned long startTime;
  unsigned long endTime;
  unsigned int theReading;
  
  byte consecutiveReadingsOn;

  if (DEBUGINFRARED) Serial.println("calcAngleFromEdgeToTape");
  
  if (leftEdge) 
    sparki.moveRight();
  else
    sparki.moveLeft();
    
  startTime = millis();

  // Loop till we have several consecutive on readings, or we've moved 45' (we shouldn't have to go that far)
  consecutiveReadingsOn = 0;
  endTime = millis();
  while ( (consecutiveReadingsOn < INFRARED_CONSECUTIVE_EDGE_READINGS) && ((endTime-startTime) < (millisFor90Degrees/2)) ) {
    if (leftEdge == true) {
      theReading = sparki.edgeLeft();      
      if (DEBUGINFRARED) Serial.print("Lft:,");
    }
    else {      
      theReading = sparki.edgeRight();
      if (DEBUGINFRARED) Serial.print("Rt:,");
    }

    if (DEBUGINFRARED) {
      unsigned long temp = endTime-startTime;
      Serial.print(",Reading:");
      Serial.print(theReading);
      Serial.print(",endTime:");
      Serial.print(endTime);
      Serial.print("<:");
      Serial.println((temp * 90)/millisFor90Degrees);
      Serial.flush();
      delay(8);
    }
      
    if (lineFlagHelper(theReading,baseReading) == true) 
      consecutiveReadingsOn++;
    else
      consecutiveReadingsOn = 0;
    delay(2);
    endTime = millis();
  }
  endTime = millis();
  sparki.moveStop();
  delay(5);
  
  // Calculate the degrees you turned, then turn back to that value
  // we'll reuse the 'theReading' variable it's not needed any longer here
  theReading = ((endTime - startTime) * 90)/millisFor90Degrees;
  if (theReading > 0) {
    if (DEBUGINFRARED) {
      Serial.print("Moving back,angle:");
      Serial.println(theReading);
    }
    if (leftEdge) 
      sparki.moveLeft(theReading);
    else
      sparki.moveRight(theReading);
    delay(5);
  }
  // If we didn't get consecutive readings we'll try one more time
  if ((consecutiveReadingsOn < INFRARED_CONSECUTIVE_EDGE_READINGS) && (isRecall == false))
    return calcAngleFromEdgeToTape(leftEdge, baseReading, millisFor90Degrees, true);
  else
    return theReading;
}


// Clear infrared readings for the argument passed in
void InfraredClass::clearInfraredAttributes(InfraredAttributes &attr2Clear) {
  attr2Clear.edgeLeft   = 0;
  attr2Clear.lineLeft   = 0;
  attr2Clear.lineCenter = 0;
  attr2Clear.lineRight  = 0;
  attr2Clear.edgeRight  = 0;
  
  attr2Clear.el_line = false;  // Set flags to default (false) value
  attr2Clear.ll_line = false;
  attr2Clear.lc_line = false;
  attr2Clear.lr_line = false;
  attr2Clear.er_line = false;

  attr2Clear.driftLeft      = false;  // State flags
  attr2Clear.driftRight     = false;
  attr2Clear.onLine         = false;
  attr2Clear.atExit         = false;
  attr2Clear.startLeftPath  = false;
  attr2Clear.startRightPath = false;
  attr2Clear.endLeftPath    = false;
  attr2Clear.endRightPath   = false;
  attr2Clear.atEntrance     = false;
}

// This routine extracts data from src and populates the array at dst
const char* InfraredClass::extractToken(char* dst, const char* src) {
  while ( *src ) {
    if ( ',' == *src ) {
      *dst = '\0';
      return ++src;
    }
    *dst++ = *src++;
  }
  *dst = '\0';
  return NULL;
}
// Return the base attributes
InfraredAttributes InfraredClass::getBaseAttributes() {
  return infraredBase;
}

// -----------------------------------------------------------------------------------------
// Gets the infrared light attributes at the current pose.  This also will use the base
// state to see if the sensor are on a line or note... it sets the flags appropriately.
//InfraredAttributes InfraredClass::getInfraredAttributesAtSensorPose() {
void InfraredClass::getInfraredAttributesAtSensorPose(InfraredAttributes &reading) {
  //InfraredAttributes reading;
  clearInfraredAttributes(reading);
  for (int i = 0; i < INFRARED_SAMPLE_SIZE; i++) {
    reading.edgeLeft   += (sparki.edgeLeft()/INFRARED_SAMPLE_SIZE);   // measure the left edge IR sensor
    reading.lineLeft   += (sparki.lineLeft()/INFRARED_SAMPLE_SIZE);   // measure the left IR sensor
    reading.lineCenter += (sparki.lineCenter()/INFRARED_SAMPLE_SIZE); // measure the center IR sensor
    reading.lineRight  += (sparki.lineRight()/INFRARED_SAMPLE_SIZE);  // measure the right IR sensor
    reading.edgeRight  += (sparki.edgeRight()/INFRARED_SAMPLE_SIZE);  // measure the right edge IR sensor
    delay(INFRARED_DELAY_BETWEEN_SAMPLES);
  }
  
  reading.el_line = lineFlagHelper(reading.edgeLeft, infraredBase.edgeLeft);  // Set flags
  reading.ll_line = lineFlagHelper(reading.lineLeft, infraredBase.lineLeft);
  reading.lc_line = lineFlagHelper(reading.lineCenter, infraredBase.lineCenter);
  reading.lr_line = lineFlagHelper(reading.lineRight, infraredBase.lineRight);
  reading.er_line = lineFlagHelper(reading.edgeRight, infraredBase.edgeRight);
  return;
  //return reading;
}

// Return the tape width
float InfraredClass::getLineWidth() {
  if (lineWidthInMM == 0) {
    return INFRARED_LINE_WIDTH;
  }
  else {
    return ((float)lineWidthInMM / 10.0);
  }
}

// Return the pose of the center infrared light detector
Pose InfraredClass::getPoseOfCenterSensor() {
  // The pose is the current pose but we add in the length that the center sensor is ahead of the robots denter
  return localizationObj->calculatePose(localizationObj->getPose(), localizationObj->getCurrentAngle(), INFRARED_SENSOR_FORWARD_OF_CENTER);
}
 

// Helper routine to return a bool (true) if the 
bool InfraredClass::lineFlagHelper(const int &currentReading, const int &baseReading) {
  if (baseReading < INFRARED_MIN_READING) {
    return false;
  }
  // We got a reading; if it's more than the the threshold amount, NOTE: did arithmatic this
  // way to ensure there's not an overflow.. it's basically saying the difference is greater than
  // N% of the base
  if (DEBUGINFRARED) {
    Serial.print("lFH:");
    Serial.print(baseReading);
    Serial.print(",");
    Serial.print(currentReading);
    Serial.print(",");
  }
  if ((baseReading - currentReading) > (baseReading * INFRARED_LINE_THRESHOLD)) {
    if (DEBUGINFRARED) Serial.println("T"); 
    return true;
  }
  else {
    if (DEBUGINFRARED)  Serial.println("F");
    return false;
  }
}

// -------------------------------------------------------------------------------------------------
// Little routine to return bool if we're on an intersection... both edge readings are on
bool InfraredClass::onIntersection(const int &baseReading) {
  return (lineFlagHelper(sparki.edgeLeft(),baseReading) || lineFlagHelper(sparki.edgeRight(),baseReading));
}


// -------------------------------------------------------------------------------------------------
// Little routine to return bool if we're on an intersection... both edge readings are on
bool InfraredClass::onLine(const int &baseReading) {
  return (lineFlagHelper(sparki.lineLeft(),baseReading) || lineFlagHelper(sparki.lineCenter(),baseReading) ||
                                                               lineFlagHelper(sparki.lineRight(),baseReading));
}


// This method takes in a char* and counts how many parms there are and what the length of the max
// parameter is
void InfraredClass::parmCountAndLength(const char* str_data, unsigned int& num_params, unsigned int& theLen) {
  unsigned int t_max    = 0;
  const char* src = str_data;
  const char* p   = nullptr;
  char  ch;
  while ( (ch = *src++) ) {
    t_max++;  
    if ( ',' == ch ) {
      p = src;
      num_params++;
      if ( t_max > theLen ) {
        theLen  = t_max;
        t_max  = 0;
      }
    }
  }
  
  const unsigned int count_remaining = strlen(p);
  if ( count_remaining ) {
    num_params++;
    if ( count_remaining > theLen ) {
      theLen = count_remaining;
    }
  }
}


// Read a character (an int) from the serial port, we'll wait 10 seconds
int InfraredClass::readFromSerialPort() {
  int theCnt = 0;
  if (DEBUGINFRARED) sparki.RGB(RGB_RED);
  while (theCnt < 10000) { 
    if (Serial.available() > 0) {
      if (DEBUGINFRARED) sparki.RGB(RGB_GREEN);  
      return (int)Serial.read();
    }
    theCnt +=5;
    delay(5);
  }
  return -1;
}
 

// -----------------------------------------------------------------------------------------
// Right now this just gets the one reading... but thinking this will be expaned to move
// around, take readings and average them out.
void InfraredClass::setInfraredBaseReadings() {
  //infraredBase = getInfraredAttributesAtSensorPose();
  getInfraredAttributesAtSensorPose(infraredBase);
}

// ==========================================================================================
// Methods predominantly for debugging... they show values 
// ==========================================================================================

// -----------------------------------------------------------------------------------------
// Helper method to show light attributes, since I don't know the angle the light attributes 
// where taken you should pass that in
void InfraredClass::showInfraredAttributes(char *msgStr, const InfraredAttributes &attr, const Pose &poseOfCenterSensor, const bool &isGoalPosition) {
  #if USE_LCD 
    sparki.print("InfraRed,");
    sparki.println(msgStr);
    sparki.print(",el,");
    sparki.print(attr.edgeLeft);
    sparki.print(",ll,");
    sparki.print(attr.lineLeft);
    sparki.print(",lc,");
    sparki.print(attr.lineCenter);
    sparki.print(",lr,");
    sparki.print(attr.lineRight);
    sparki.print(",er,");
    sparki.println(attr.edgeRight);
    sparki.updateLCD();
  #else
    Serial.print(F("IR,"));
    Serial.print(msgStr);

    // Show the pose before you put out the values
    Serial.print(F(",x,"));
    Serial.print(poseOfCenterSensor.xPos);
    Serial.print(F(",y,"));
    Serial.print(poseOfCenterSensor.yPos);
    Serial.print(F(",<,"));
    Serial.print(poseOfCenterSensor.angle);
    
    // Now output sensor values
    Serial.print(F(",el,"));
    Serial.print(attr.edgeLeft);
    Serial.print(F(",ll,"));
    Serial.print(attr.lineLeft);
    Serial.print(F(",lc,"));
    Serial.print(attr.lineCenter);
    Serial.print(F(",lr,"));
    Serial.print(attr.lineRight);
    Serial.print(F(",er,"));
    Serial.print(attr.edgeRight);

    Serial.print(F(",ell,"));
    Serial.print(attr.el_line);
    Serial.print(F(",lll,"));
    Serial.print(attr.ll_line);
    Serial.print(F(",lcl,"));
    Serial.print(attr.lc_line);
    Serial.print(F(",lrl,"));
    Serial.print(attr.lr_line);
    Serial.print(F(",erl,"));
    Serial.print(attr.er_line);
      
    Serial.print(F(",sdl,"));  
    Serial.print(attr.driftLeft);      
    Serial.print(F(",sdr,"));
    Serial.print(attr.driftRight);     
    Serial.print(F(",sol,"));
    Serial.print(attr.onLine);         
    Serial.print(F(",sae,"));
    Serial.print(attr.atExit);         
    Serial.print(F(",slp,"));
    Serial.print(attr.startLeftPath);  
    Serial.print(F(",srp,"));
    Serial.print(attr.startRightPath); 
    Serial.print(F(",sel,"));
    Serial.print(attr.endLeftPath);    
    Serial.print(F(",ser,"));
    Serial.print(attr.endRightPath);   
    Serial.print(F(",sas,"));
    Serial.print(attr.atEntrance);
    
    Serial.print(F(",sgl,"));          // Is a goal node
    Serial.println(isGoalPosition);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif  
}

// Check the state between two sets of infrared attributes, if any of the line flags don't match the
// last values then return 'true' meaning the state changed, otherwise return false.
// NOTE: This updates the attributes for currAttr so be careful
bool InfraredClass::stateChanged(InfraredAttributes &currAttr, const InfraredAttributes &priorAttr) {
  if (DEBUGINFRARED) {
    showInfraredAttributes("StateChgCurr",currAttr, getPoseOfCenterSensor(), false);
    showInfraredAttributes("StateChgPrio",priorAttr, getPoseOfCenterSensor(), false);
  }
  
  // We're drifting if the outside sensor is on and the other two are off
  currAttr.driftLeft  = (currAttr.ll_line == false && currAttr.lc_line == false && currAttr.lr_line == true);
  currAttr.driftRight = (currAttr.ll_line == true && currAttr.lc_line == false && currAttr.lr_line == false);

  // We're on a line if one of the sensors is on and we're not 'drifting'
  if (currAttr.driftLeft == false && currAttr.driftRight == false)
    currAttr.onLine = (currAttr.ll_line || currAttr.lc_line || currAttr.lr_line);

  // We're at an exit if the prior attributes were on a line (not turn path) and all our line indicators are off
  if (priorAttr.el_line == false && priorAttr.er_line == false && (priorAttr.ll_line || priorAttr.lc_line || priorAttr.lr_line)) {
    currAttr.atExit = (currAttr.el_line == false && currAttr.ll_line == false && currAttr.lc_line == false && 
                       currAttr.lr_line == false && currAttr.er_line == false);
  }
 
  // At start of left path if the light edge light went on
  currAttr.startLeftPath = (priorAttr.el_line == false && currAttr.el_line == true);

  // Start of right path is similar but for right light
  currAttr.startRightPath = (priorAttr.er_line == false && currAttr.er_line == true);

  // End paths are opposite of above... light was on and now off 
  currAttr.endLeftPath  = (priorAttr.el_line == true && currAttr.el_line == false);
  currAttr.endRightPath = (priorAttr.er_line == true && currAttr.er_line == false);
 
  // At entrance, if all the prior line indicators are off and one of our center line indicators is on (but edge ones are off)
  if (priorAttr.el_line == false && priorAttr.ll_line == false && priorAttr.lc_line == false &&  priorAttr.lr_line == false && priorAttr.er_line == false) {
    currAttr.atEntrance = (currAttr.el_line == false && currAttr.er_line == false && (currAttr.ll_line || currAttr.lc_line || currAttr.lr_line));
  }

  // Return indicator to caller if any state requires attention... basically the only one we don't notify on
  // is when we'r 'OnLine'
  if (currAttr.driftLeft || currAttr.driftRight || currAttr.atExit || currAttr.startLeftPath || currAttr.startRightPath || 
      currAttr.endLeftPath || currAttr.endRightPath || currAttr.atEntrance) {
    return true;
  } 
  else {
    return false;
  }    
}

// Wait for instructions on the serial port... we'll continue in this loop till
// we get the 'serial termination' character (|)
bool InfraredClass::waitForInstructions(QueueArray<InfraredInstructions> &queueOfInstructions) {

  // Clear line in case anything on it
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      Serial.read();
      delay(2);
    }
  }

  Serial.println(F("IR,INS"));  // Tells python you want instructions
  Serial.flush();
  //localizationObj->writeMsg2Serial("IR,INS");

  // Python will first tell us how many characters will follow we'll use that to allocate the array
  // The routine below will wait up to 10 seconds
  int bytes2Read = 0;
  while ((bytes2Read != 1) && (bytes2Read >= 0)) {
    bytes2Read = readFromSerialPort();
  }
  // We processed trigger get the count of bytes to get
  if (bytes2Read == 1)
    bytes2Read = readFromSerialPort();
  
  if (DEBUGINFRARED == false) { Serial.print(F("bytes2Read:")); Serial.println(bytes2Read); }
  
  char *str_data = NULL;
  if (bytes2Read > 0) {
    if (DEBUGINFRARED) Serial.println(F("w1"));
    char inputBuffer[bytes2Read+1];
    int indexPos       = 0;
    int maxLen         = 0;
    int num_parameters = 0;
    int currLen        = 0;
    int valueRead;
    while (indexPos < bytes2Read) {
      valueRead = readFromSerialPort();
      if (valueRead < 0) {
        // Didn't get a value... we'll exit
        bytes2Read = 0; 
      }
      else {        
        inputBuffer[indexPos] = (char)valueRead;
        if (inputBuffer[indexPos] == ',') {
          if (currLen > maxLen) 
            maxLen = currLen;
          currLen = 0;
          num_parameters++;
        }
        else currLen++;
        indexPos++;
      }
    }
    inputBuffer[indexPos] = '\0';  // null terminate it
 
    // Take any junk off port
    delay(2);
    while (Serial.available() > 0) {
      Serial.read();
      delay(2);
    }

    // bytes2Read is set to zero if encountered a problem
    if (bytes2Read > 0) {
      if (DEBUGINFRARED) Serial.println(F("w2")); 
      // Process the last record from above... if currLen is > 0 we need to process it
      if (currLen > 0) {
        num_parameters++;
        if (currLen > maxLen) 
          maxLen = currLen;
      }
      // Assign pointer to address of array (same as str_data = &inputBuffer[0];)
      str_data = inputBuffer;
      
      // Now put into string array
      char szParams[num_parameters][maxLen+1];
  
      // This setups the 0'the row and sets pnext to point to the next address
      char* pnext = (char*)extractToken(&szParams[0][0], str_data);
      for (indexPos = 1; indexPos < num_parameters; indexPos++) {  
        pnext = (char*)extractToken(&szParams[indexPos][0], pnext);
      }
    
      if (DEBUGINFRARED) {
        for ( size_t i = 0; i < num_parameters; i++ ) {
           Serial.print("szParams["); Serial.print(i); Serial.print("]:");
           Serial.println(szParams[i]);
        }
      }
    
      // Put the instructions into the queue
      indexPos = 0;
      InfraredInstructions theInstructions;
      while (indexPos < num_parameters) {
        switch( indexPos % 7) {
          case 0:
            theInstructions.instruction = szParams[indexPos][0];
            if (DEBUGINFRARED) {
              Serial.print("ins:");
              Serial.println(theInstructions.instruction);
            }
            break;
          case 2: // x
            theInstructions.pose.xPos = atof(szParams[indexPos]);
            if (DEBUGINFRARED) {
              Serial.print("xPos:");
              Serial.println(theInstructions.pose.xPos);
            }
            break;
          case 4: // y
            theInstructions.pose.yPos = atof(szParams[indexPos]);
            if (DEBUGINFRARED) {        
              Serial.print("yPos:");
              Serial.println(theInstructions.pose.yPos);
            }
            break;
          case 6: // angle
            theInstructions.pose.angle = atoi(szParams[indexPos]);
            if (DEBUGINFRARED) {        
              Serial.print("<:");
              Serial.println(theInstructions.pose.angle);
            }
            queueOfInstructions.push(theInstructions);
            break;
        }
        indexPos++;
      }
      return true;
    }
    else {
      if (DEBUGINFRARED) Serial.println(F("w8"));
      return false;
    }
  }
  else {
    if (DEBUGINFRARED) Serial.println(F("w9"));
    return false; // Serial.println("No records to read on serial port");
  }

  // Old code here from debuggin issues with communicating with sparky... appears was memory related
  // more than anything
  // Serial.flush();
  // delay(100);                                        // CHANGE THESE TO CONSTANTS 
  
  // Serial.print("#");
  // Serial.println(Serial.available());
  // while (Serial.available() == 0) {
  //   delay(300);
  //   sparki.beep();
  // }
  
  // String rtnString = Serial.readStringUntil('|');
  // if (DEBUGINFRARED) {
  //   Serial.print("r$:");
  //   Serial.println(rtnString);
  // }

  // Format of the data returned is similar to comment on right, we get -1 values for angles on GOTO (M)
  // (the X is eXplore, Q is done/quit, G is Goal)
  // since it's not relevant... we get -1.0, -1,0, -1 on the DONE verb too
  
  /*
  const char*  str_data = rtnString.c_str();  //"M,x,9.2,y,4.3,<,-1,X,x,4.52,y,5.21,34,<,90"
  size_t num_parameters = 0;
  size_t len_parameter = 0;
    
  parmCountAndLength(str_data, num_parameters, len_parameter);
  */
  
  
  /*
  String rtnString = "";
  bool keepReading = true;
  while (keepReading) {
    delay(DELAY_FOR_SERIAL_COMM);  
    if (Serial.available() > 0)  {
      char theChar = (char)Serial.read();
      if (theChar == '\') {                   // CHANGE THIS TO CONSTANTE
        keepReading = false
      }
      else {
        rtnString += theChar;
      }
    }
  }
  */
  //return *str_data;
}

