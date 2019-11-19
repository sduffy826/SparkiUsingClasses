#include "infraredClass.h"
      
// =========================================================================================
//      Constructor(s)
// =========================================================================================
InfraredClass::InfraredClass(LocalizationClass &localizationObject, MovementsClass &movementsObject) {
  localizationObj = &localizationObject;
  movementsObj    = &movementsObject;
  lineWidthInMM   = 0;
  clearInfraredAttributes(infraredBase);
}

// =========================================================================================
//      Methods 
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// We've drifted, try to straighten back out
void InfraredClass::adjustForDrifting(const bool &driftingLeft) {
  if (driftingLeft) {
    if (DEBUGINFRARED) localizationObj->writeMsg2Serial("IA,AdjustLeftDrift");
    movementsObj->turnLeft(90 - INFRARED_DRIFT_ADJUSTMENT_DEGREES);
    while (movementsObj->moveBackward((getLineWidth()/2.0), ULTRASONIC_MIN_SAFE_DISTANCE, false));
    movementsObj->turnRight(90);
  }
  else {
    if (DEBUGINFRARED) localizationObj->writeMsg2Serial("IA,AdjustRightDrift");
    movementsObj->turnRight(90 - INFRARED_DRIFT_ADJUSTMENT_DEGREES);
    while (movementsObj->moveBackward((getLineWidth()/2.0), ULTRASONIC_MIN_SAFE_DISTANCE, false));
    movementsObj->turnLeft(90);
  }
}


// -----------------------------------------------------------------------------------------
// Assigning structs is just reference assignment, this does the byname... didn't overload assignment
// as not sure if arduino c++ handles it and this was easy
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

// Return the base attributes
InfraredAttributes InfraredClass::getBaseAttributes() {
  return infraredBase;
}

// -----------------------------------------------------------------------------------------
// Gets the infrared light attributes at the current pose.  This also will use the base
// state to see if the sensor are on a line or note... it sets the flags appropriately.
InfraredAttributes InfraredClass::getInfraredAttributesAtCurrentPose() {
  InfraredAttributes reading;
  clearInfraredAttributes(reading);
  for (int i = 0; i < INFRARED_SAMPLE_SIZE; i++) {
    reading.edgeLeft   += (sparki.edgeLeft()/INFRARED_SAMPLE_SIZE);   // measure the left edge IR sensor
    reading.lineLeft   += (sparki.lineLeft()/INFRARED_SAMPLE_SIZE);   // measure the left IR sensor
    reading.lineCenter += (sparki.lineCenter()/INFRARED_SAMPLE_SIZE); // measure the center IR sensor
    reading.lineRight  += (sparki.lineRight()/INFRARED_SAMPLE_SIZE);  // measure the right IR sensor
    reading.edgeRight  += (sparki.edgeRight()/INFRARED_SAMPLE_SIZE);  // measure the right edge IR sensor
    delay(8);
  }
  
  reading.el_line = lineFlagHelper(reading.edgeLeft, infraredBase.edgeLeft);  // Set flags
  reading.ll_line = lineFlagHelper(reading.lineLeft, infraredBase.lineLeft);
  reading.lc_line = lineFlagHelper(reading.lineCenter, infraredBase.lineCenter);
  reading.lr_line = lineFlagHelper(reading.lineRight, infraredBase.lineRight);
  reading.er_line = lineFlagHelper(reading.edgeRight, infraredBase.edgeRight);
  
  return reading;
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
 

// Helper routine to return a boolean (true) if the 
boolean InfraredClass::lineFlagHelper(const int &currentReading, const int &baseReading) {
  if (baseReading < INFRARED_MIN_READING) {
    return false;
  }
  // We got a reading; if it's more than the the threshold amount, NOTE: did arithmatic this
  // way to ensure there's not an overflow.. it's basically saying the difference is greater than
  // N% of the base
  if ((baseReading - currentReading) > (baseReading * INFRARED_LINE_THRESHOLD)) {
    return true;
  }
  else {
    return false;
  }
}


// -----------------------------------------------------------------------------------------
// Right now this just gets the one reading... but thinking this will be expaned to move
// around, take readings and average them out.
void InfraredClass::setInfraredBaseReadings() {
  infraredBase = getInfraredAttributesAtCurrentPose();
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
    Serial.print("IR,");
    Serial.print(msgStr);

    // Show the pose before you put out the values
    Serial.print(",x,");
    Serial.print(poseOfCenterSensor.xPos);
    Serial.print(",y,");
    Serial.print(poseOfCenterSensor.yPos);
    Serial.print(",<,");
    Serial.print(poseOfCenterSensor.angle);
    
    // Now output sensor values
    Serial.print(",el,");
    Serial.print(attr.edgeLeft);
    Serial.print(",ll,");
    Serial.print(attr.lineLeft);
    Serial.print(",lc,");
    Serial.print(attr.lineCenter);
    Serial.print(",lr,");
    Serial.print(attr.lineRight);
    Serial.print(",er,");
    Serial.print(attr.edgeRight);

    Serial.print(",ell,");
    Serial.print(attr.el_line);
    Serial.print(",lll,");
    Serial.print(attr.ll_line);
    Serial.print(",lcl,");
    Serial.print(attr.lc_line);
    Serial.print(",lrl,");
    Serial.print(attr.lr_line);
    Serial.print(",erl,");
    Serial.print(attr.er_line);
      
    Serial.print(",sdl,");  
    Serial.print(attr.driftLeft);      
    Serial.print(",sdr,");
    Serial.print(attr.driftRight);     
    Serial.print(",sol,");
    Serial.print(attr.onLine);         
    Serial.print(",sae,");
    Serial.print(attr.atExit);         
    Serial.print(",slp,");
    Serial.print(attr.startLeftPath);  
    Serial.print(",srp,");
    Serial.print(attr.startRightPath); 
    Serial.print(",sel,");
    Serial.print(attr.endLeftPath);    
    Serial.print(",ser,");
    Serial.print(attr.endRightPath);   
    Serial.print(",sas,");
    Serial.print(attr.atEntrance);
    
    Serial.print(",sgl,");          // Is a goal node
    Serial.println(isGoalPosition);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif  
}

// Check the state between two sets of infrared attributes, if any of the line flags don't match the
// last values then return 'true' meaning the state changed, otherwise return false.
// NOTE: This updates the attributes for currAttr so be careful
bool InfraredClass::stateChanged(InfraredAttributes &currAttr, const InfraredAttributes &priorAttr) {
  if (DEBUGINFRARED) {
    showInfraredAttributes("StateChg,curr,",currAttr, getPoseOfCenterSensor(), false);
    showInfraredAttributes("StateChg,prio,",priorAttr, getPoseOfCenterSensor(), false);
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
String InfraredClass::waitForInstructions() {
  // This will wait for the instructions from the computer
  localizationObj->writeMsg2Serial("IR,INS");
  
  delay(1000);                                        // CHANGE THESE TO CONSTANTS 
  String rtnString = Serial.readStringUntil('|');
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
  return rtnString;
}

