#include "movementsClass.h"

MovementsClass::MovementsClass() { }

MovementsClass::MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject) {
  ultrasonicObj = ultrasonicObject;
  localizationObj = localizationObject;
  determineWorldObj = determineWorldObject;
}


void MovementsClass::initMovements() {
  ultrasonicObj.positionServo(0);
  movementState.amMoving  = false;
  movementState.startTime = 0;
}

// Routine to return elapsed time... older version below in case you need it
unsigned int MovementsClass::getElapsed() {
  return millis() - movementState.startTime;
}
/* unsigned int getElapsed(bool setTime) { 
  unsigned int temp = millis() - movementState.startTime;
  if (setTime) {
    // if want to set the time increment startTime by the elapsed amount
    // could have saved millis but that'd introduce another variable and want to save memory
    movementState.startTime = temp + movementState.startTime;
  }
  return temp;
} */

// Return number of milliseconds required to cover a distance.
unsigned int MovementsClass::getMillisToGetThere(float distanceInCM) {
  return (distanceInCM / VELOCITY_CM_P_SEC) * 1000;
}

// Routine to signify we're moving
void MovementsClass::startMovingForward() {
  if ( movementState.amMoving == false ) {
    movementState.amMoving = true;
    movementState.startTime = millis();
    sparki.moveForward();
  }
}

// Return the distance traveled so far
float MovementsClass::getDistanceTraveledSoFar() {
  if ( movementState.amMoving == true ) {
    return ((getElapsed() * VELOCITY_CM_P_SEC) / 1000.0);
  }
  return 0.0;
}

// Stop moving and return the distance traveled
void MovementsClass::stopMoving() {
  // Set your position, the angle delta is 0 since we're going in a straight line
  localizationObj.setNewPosition(getDistanceTraveledSoFar(), 0.0);
  sparki.moveStop();
  movementState.amMoving = false;
}

// Handles moving forward a given distance, second parm is the min distance you can be within of a wall
// Intent is that this will be called continuously... it doesn't block... it'll start moving if sparki isn't
// and it'll stop when we've reached destination or wall.
boolean MovementsClass::moveForward(float distanceToTravel, float minAllowedDistanceToObstacle) {
  if ( movementState.amMoving == true ) {
    if ((getDistanceTraveledSoFar() >= distanceToTravel) || (ultrasonicObj.distanceAtAngle(0) <= minAllowedDistanceToObstacle)) {  // done moving
      stopMoving();
    } 
  }
  else {
    if ((getMillisToGetThere(distanceToTravel) <= 0.0) || (ultrasonicObj.distanceAtAngle(0) <= minAllowedDistanceToObstacle)) {  // cant move
      sparki.beep();
    }
    else {
      startMovingForward();
    }
  } 
  return movementState.amMoving;
}  

// Turn left or right byt certain degrees and set your current orientation angle
void MovementsClass::turnLeft(byte degrees) {
  sparki.moveLeft(degrees);
  localizationObj.setCurrentAngle(localizationObj.calculateRealAngleWithAdjustment(-(float)degrees));
}
  
void MovementsClass::turnRight(byte degrees) {
  sparki.moveRight(degrees);
  localizationObj.setCurrentAngle(localizationObj.calculateRealAngleWithAdjustment((float)degrees));
}

int MovementsClass::getClosest90Angle() {
  return ((((int)(localizationObj.getCurrentAngle()+44)/90)*90)%360);
}

void MovementsClass::turnToAngle(int theAngle) {
  int shortestPath = (int)localizationObj.getShortestAngleDeltaToGetToOrientation((float)theAngle);
  if (shortestPath < 0) {
    turnLeft(-shortestPath);
  }
  else {
    turnRight(shortestPath);
  }
}

// Turn to specific angle and return distance in front of you :)
int MovementsClass::getDistanceAtAngle(int angle) {
  turnToAngle(angle);
  return ultrasonicObj.distanceAtAngle(0);
}

// Convenience routine
void MovementsClass::turnToZero() {
  turnToAngle(0);
}

// Another convenience routine
void MovementsClass::turnTo90ClosestDegreeOrientation() {
  turnToAngle(getClosest90Angle());
}

// -------------------------- 
// Right now it only moves to the left, need to make
// it smart enough to turn right too
void MovementsClass::followWall() {
  int saveX, saveY;
  int theDistance;
  byte numTurns;
  // Face the wall to the right
  turnRight(90);
  theDistance = ultrasonicObj.distanceAtAngle(0);
  // Move forward distance to wall, less turn radius, second parm is min distance
  // to stay away from wall.
  while (moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE));
  saveX = localizationObj.getCurrentXPositionInCM();
  saveY = localizationObj.getCurrentYPositionInCM();

  // Min turns required to go around a rectangular space is 5, we won't make more than 20 and
  // we will stop when back at the original location
  numTurns = 0;
  while ((numTurns < 5) || 
         (numTurns < 20 &&  
          saveX != localizationObj.getCurrentXPositionInCM() &&
          saveY != localizationObj.getCurrentYPositionInCM())) {
    turnLeft(90);
    numTurns++;
    theDistance = ultrasonicObj.distanceAtAngle(0);
    while (moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE)) {
      localizationObj.showLocation();
      delay(100);
    }
  } 
}

void MovementsClass::showTurnRadius() {
  // Write to the lcd or the serial device 
  #if USE_LCD 
    sparki.print("Sparki turn radius: ");
    sparki.println(getTurnRadius());
    sparki.updateLCD();
    delay(5000);
  #else
    Serial.print("MR,");
    Serial.println(TURN_RADIUS);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}

void MovementsClass::showWallMovements() {
  #if USE_LCD 
    sparki.print("Sparki turn radius: ");
    sparki.println(getTurnRadius());
    sparki.updateLCD();
    delay(5000);
  #else
    Serial.print("MW,");
    // Serial.print(distanceTraveledSoFar());
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}
