#include "movementsClass.h"

#define DEBUGWALL true

// ========================================================================
//     Constructors
// ========================================================================
MovementsClass::MovementsClass() { }

// Constructor pass in the objects needed 
MovementsClass::MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject) {
  ultrasonicObj = &ultrasonicObject;
  localizationObj = &localizationObject;
  determineWorldObj = &determineWorldObject;
  initMovements();
}

// ========================================================================
//     Methods 
// ========================================================================

// --------------------------------------------------------------------------------------
// This SHOULD not be called if we're on a wall opening, cause values get reset after stopping
float MovementsClass::adjustDistanceToWall(const float &desiredDistance, const float &currentWallDistance) {
  stopMoving();
  turnRight(90);  // turnLogic
  if (desiredDistance > currentWallDistance) {  // Too close backup
    while (moveBackward(desiredDistance-currentWallDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  else { 
    while (moveForward(currentWallDistance-desiredDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  turnLeft(90);
  float newDistanceForward = ultrasonicObj->getFreeSpaceInFrontOfGripper(0);
  
  #if DEBUGWALL
    Serial.print("In adjustDistanceToWall");
    Serial.print(" desired: ");
    Serial.print(desiredDistance);
    Serial.print(" current: ");
    Serial.print(currentWallDistance);
    Serial.print(" newDistanceAfterAdjustment");
    Serial.println(newDistanceForward);
  #endif
  return newDistanceForward;
}

// --------------------------------------------------------------------------------------
// Right now it's following the right wall, if want left wall then change the 'turnRight' to left and 
// vica versa... I marked commend on lines neededing change (look at 'turnLogic' comment).
// it smart enough to turn right too
void MovementsClass::followWall() {
  int saveX, saveY;
  float theDistance;
  float startWallDistance, lastWallDistance, widthOfOpening;
  byte numTurns;
  // Face the wall to the right
  turnRight(90);  // turnLogic
  theDistance = ultrasonicObj->getFreeSpaceInFrontOfGripper(0);

  #if DEBUGWALL
    Serial.print("D1 ");
    Serial.println(theDistance);
  #endif
  // Move forward distance to wall, less turn radius, second parm is min distance
  // to stay away from wall.
  while (moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  saveX = localizationObj->getCurrentXPositionInCM();
  saveY = localizationObj->getCurrentYPositionInCM();

  #if DEBUGWALL
    Serial.print("S1 ");
    Serial.print(saveX);
    Serial.print(",");
    Serial.println(saveY);
  #endif
  // Min turns required to go around a rectangular space is 5, we won't make more than 20 and
  // we will stop when back at the original location
  numTurns = 0;
  while ((numTurns < 5) || 
         (numTurns < 20 &&  
          saveX != localizationObj->getCurrentXPositionInCM() &&
          saveY != localizationObj->getCurrentYPositionInCM())) {
    turnLeft(90);  // turnLogic
    numTurns++;
    theDistance = ultrasonicObj->getFreeSpaceInFrontOfGripper(0);
    startWallDistance = lastWallDistance = ultrasonicObj->getFreeSpaceOnRight();    // turnLogic
    #if DEBUGWALL
      Serial.print("D2 ");
      Serial.print(theDistance);
      Serial.print(" SWD: ");
      Serial.println(startWallDistance);
    #endif

    // We'll move forward till we are at next wall in front of us, or we can go thru a wall opening
    while (moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE, false)) {
      // Calculate the width of the wall opening... the routine below keeps track of that for us
      // NOTE: 'theDistance' argument is passed by reference... it's reset when we need to adjust our
      // distance to the wall next to us (we stop, fix wall and then recalculate new distance to end wall)
      widthOfOpening = wallOpeningDistance(theDistance, startWallDistance, lastWallDistance);

      if (widthOfOpening > MINWALLOPENINGWIDTH) {
        #if DEBUGWALL
          Serial.print("LargeOpening");
          Serial.print(" width: ");
          Serial.println(widthOfOpening);
        #endif      
        // We can fit thru the opening, backup 1/2 the distance of the opening, turn right and move forward again :)
        stopMoving();
        localizationObj->showLocation();  // For debugging show the location
      
        while (moveBackward((widthOfOpening / 2.0), 0.0, false));  // backup 1/2 distance, don't care about min distance (2,3 args)
        turnRight(90);                                             // turn right
        theDistance = ultrasonicObj->getFreeSpaceInFrontOfGripper(0);
      }
      // had delay(100) but removed it... see how it works.
    }
  } 
}
// --------------------------------------------------------------------------------------
/*
void MovementsClass::handleWallChange(int startWallDistance, int currWallDistance, int lastWallDistance) {

  if (currWallDistance > WALLOPENINGDISTANCE)


  
  movementState.wallOpening = false;  // indicator for when there's an opening in the wall
  movementState.rightWall = true;  // Means want to follow wall with wall on the right
  
}


  
  if ((currWallDistance - lastWallState) > 1) {  // Wall state changed
     handleWallChange(startWallDistance, currWallDistance, true);  // turnLogic
     lastWallState = currWallState;
  }
*/

// --------------------------------------------------------------------------------------
int MovementsClass::getClosest90Angle() {
  return (( ((localizationObj->getCurrentAngle()+44)/90) * 90) % 360);
}

// --------------------------------------------------------------------------------------
// Turn to specific angle and return distance in front of you :)
float MovementsClass::getDistanceAtAngle(const int &angle) {
  turnToAngle(angle);
  return ultrasonicObj->getFreeSpaceInFrontOfGripper(0);
}

// --------------------------------------------------------------------------------------
// Return distance travelled for duration
float MovementsClass::getDistanceTraveledForTime(const int &milliseconds) {
  return ((milliseconds * VELOCITY_CM_P_SEC) / 1000.0);
}

// --------------------------------------------------------------------------------------
// Return the distance traveled so far
float MovementsClass::getDistanceTraveledSoFar() {
  if ( movementState.amMoving == true ) {
    return getDistanceTraveledForTime(getElapsedSinceStartedMoving());
  }
  return 0.0;
}

// --------------------------------------------------------------------------------------
// Routine to return elapsed time... older version below in case you need it
unsigned int MovementsClass::getElapsedSinceStartedMoving() {
  return millis() - movementState.startTime;
}

// --------------------------------------------------------------------------------------
// THIS IS COMMENTED OUT, JUST LEFT FOR FUTURE REF
/* unsigned int getElapsedSinceStartedMoving(bool setTime) { 
  unsigned int temp = millis() - movementState.startTime;
  if (setTime) {
    // if want to set the time increment startTime by the elapsed amount
    // could have saved millis but that'd introduce another variable and want to save memory
    movementState.startTime = temp + movementState.startTime;
  }
  return temp;
} */

// --------------------------------------------------------------------------------------
// Return number of milliseconds required to cover a distance.
unsigned int MovementsClass::getMillisToGetThere(const float &distanceInCM) {
  return (distanceInCM / VELOCITY_CM_P_SEC) * 1000;
}

// --------------------------------------------------------------------------------------
// Initialize movement attributes
void MovementsClass::initMovements() {
  ultrasonicObj->positionServo(0);
  movementState.amMoving  = false;
  movementState.wallOpening = false;  // indicator for when there's an opening in the wall
  movementState.rightWall = true;  // Means want to follow wall with wall on the right
  
  movementState.startTime = 0;
  movementState.elapsedStateChange = 0;
  movementState.timeSinceLocalized = 0;
}

// --------------------------------------------------------------------------------------
// Handles moving backward a given distance, second parm is the min distance you can be within of a wall
// Intent is that this will be called continuously... it doesn't block... it'll start moving if sparki isn't
// and it'll stop when we've reached destination or wall.
boolean MovementsClass::moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance) {
  if ( movementState.amMoving == true ) {
    setLocalizationPosition(movementState.inReverse);
    if (getDistanceTraveledSoFar() >= distanceToTravel) {  // done moving
      stopMoving();
    }
    else {
      if (checkFrontDistance && (ultrasonicObj->getFreeSpaceInFrontOfGripper(0) >= minAllowedDistanceToObstacle)) {  // done moving
        stopMoving();
      }
    }
  }
  else {
    if ((getMillisToGetThere(distanceToTravel) <= 0.0) || (checkFrontDistance && (ultrasonicObj->getFreeSpaceInFrontOfGripper(0) >= minAllowedDistanceToObstacle))) {  // cant move
      sparki.beep();
    }
    else {
      startMoving(false);  // False means go in reverse
    }
  } 
  return movementState.amMoving;
}  

// --------------------------------------------------------------------------------------
// Similar to above but moves foward.
boolean MovementsClass::moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance) {
  if ( movementState.amMoving == true ) {
    setLocalizationPosition(movementState.inReverse);
    if (getDistanceTraveledSoFar() >= distanceToTravel) {  // done moving
      stopMoving();
    }
    else {
      if (checkFrontDistance && (ultrasonicObj->getFreeSpaceInFrontOfGripper(0) <= minAllowedDistanceToObstacle)) {  // done moving
        stopMoving();
      }
    }
  }
  else {
    if ((getMillisToGetThere(distanceToTravel) <= 0.0) || (checkFrontDistance && ultrasonicObj->getFreeSpaceInFrontOfGripper(0) <= minAllowedDistanceToObstacle)) {  // cant move
      sparki.beep();
    }
    else {
      startMoving(true);
    }
  } 
  return movementState.amMoving;
} 

// --------------------------------------------------------------------------------------
// Crude, put in logic for obstacles
void MovementsClass::moveToPose(const Pose &targetPose) {
  
  Pose currentPose = localizationObj->getPose();
  
  int targetAngle = localizationObj->calculateAngleBetweenPoints(currentPose.xPos, currentPose.yPos, targetPose.xPos, targetPose.yPos);
  
  // Turn to specific angle and return distance in front of you :)
  float openSpace = getDistanceAtAngle(targetAngle);
  float distance2Move = localizationObj->distanceBetweenPoses(currentPose, targetPose);
  if ((float)openSpace < distance2Move) {
    distance2Move = (float)openSpace;
  }
  while (moveForward(distance2Move, ULTRASONIC_MIN_SAFE_DISTANCE, true));
}

// --------------------------------------------------------------------------------------
// We want localization data to be accurate so this routine is called repeatedly to 
// update the pos based on last time we localized
void MovementsClass::setLocalizationPosition(const boolean &inReverse) {
  if (movementState.amMoving == true) {
    unsigned int deltaTime = millis() - movementState.timeSinceLocalized;
    // Set your position, the angle delta is 0 since we're going in a straight line
    if (inReverse) {
      localizationObj->setNewPosition(-getDistanceTraveledForTime(deltaTime), 0.0);
    }
    else {      
      localizationObj->setNewPosition(getDistanceTraveledForTime(deltaTime), 0.0);
    }
    movementState.timeSinceLocalized += deltaTime;
  }
}

// --------------------------------------------------------------------------------------
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

// --------------------------------------------------------------------------------------
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

// --------------------------------------------------------------------------------------
// Routine to signify we're moving
void MovementsClass::startMoving(const bool &goForward) {
  if ( movementState.amMoving == false ) {
    movementState.amMoving = true;
    movementState.startTime = millis();
    movementState.timeSinceLocalized = movementState.startTime;
    movementState.elapsedStateChange = 0;
    movementState.wallOpening = false;
    if (goForward) {
      sparki.moveForward();
      movementState.inReverse = false;
    }
    else {
      sparki.moveBackward();
      movementState.inReverse = true;
    }
  }
}

// --------------------------------------------------------------------------------------
// Stop moving and return the distance traveled
void MovementsClass::stopMoving() {
  // This has to be called before you say you're not moving, otherwise it won't update position
  setLocalizationPosition(movementState.inReverse);
  sparki.moveStop();
  movementState.amMoving = false;
  determineWorldObj->checkWorldCoordinates(); // We may have found new world positions, check em
}

// --------------------------------------------------------------------------------------
// Turn left or right byt certain degrees and set your current orientation angle
void MovementsClass::turnLeft(const int &degrees) {
  if (movementState.amMoving) stopMoving(); // Just in case some turkey tells us to turn in the middle 
  sparki.moveLeft(degrees);
  localizationObj->setCurrentAngle(localizationObj->calculateRealAngleWithAdjustment(-degrees));
}

// --------------------------------------------------------------------------------------  
void MovementsClass::turnRight(const int &degrees) {
  if (movementState.amMoving) stopMoving(); // Just in case some turkey tells us to turn in the middle 
  sparki.moveRight(degrees);
  localizationObj->setCurrentAngle(localizationObj->calculateRealAngleWithAdjustment(degrees));
}

// --------------------------------------------------------------------------------------
void MovementsClass::turnToAngle(const int &theAngle) {
  // We call getAngle on arg to make sure it's a positive angle we checking
  int shortestPath = localizationObj->getShortestAngleDeltaToGetToOrientation(localizationObj->getAngle(theAngle));
  if (shortestPath < 0) {
    turnLeft(-shortestPath);
  }
  else
    if (shortestPath > 0) {
      turnRight(shortestPath);
    }
}

// --------------------------------------------------------------------------------------
// Convenience routine
void MovementsClass::turnToZero() {
  turnToAngle(0);
}

// --------------------------------------------------------------------------------------
// Another convenience routine
void MovementsClass::turnToClosest90DegreeOrientation() {
  turnToAngle(getClosest90Angle());
}

// --------------------------------------------------------------------------------------
// Check wall
// NOTE NOTE NOTE: The argument distanceMovingForward is changed here, intentionally... it is new moving forward distance
// Logic
// if currentDistance from wall >= MINWALLOPENING
//   if moveState.wallOpening == false
//     moveState.wallOpening = true;
//     moveState.distanceOfOpening = 0
// else
//   if moveState.wallOpening == true
//     moveState.wallOpening = false
//     moveState.distanceOfOpening = 0
// if moveState.wallOpening
//   return the distance of the wall opening
// else
//   see if the distance you are from the wall is the desired distance
//   if it isn't then call routine to adjust the wall distance
float MovementsClass::wallOpeningDistance(float &distanceMovingForward, const float &startWallDistance, const float &lastWallDistance) {

  float currentWallDistance = ultrasonicObj->getFreeSpaceOnRight();  // turnLogic

  if (currentWallDistance > MINWALLOPENINGDEPTH) {
    if (movementState.wallOpening == false) {
      movementState.wallOpening = true;
      movementState.elapsedStateChange = getElapsedSinceStartedMoving();
    
      #if DEBUGWALL
        Serial.print("StartWallOpening");
        Serial.print(" currDist: ");
        Serial.println(currentWallDistance);
      #endif 
    }
  }
  else {
    #if DEBUGWALL
      if (movementState.wallOpening) {
        Serial.print("EndWallOpening");
        Serial.print(" currDist: ");
        Serial.println(currentWallDistance);
      }
    #endif      
    // If state was true and flipped then set indicator and new elapsed time
    if ( movementState.wallOpening == true ) {
      movementState.wallOpening = false;
      movementState.elapsedStateChange = getElapsedSinceStartedMoving();
    }
  }
  
  if (movementState.wallOpening) {
    // if at wall opening see if we should handle it... i.e turn down it
    return (getDistanceTraveledForTime(getElapsedSinceStartedMoving() - movementState.elapsedStateChange));
  }
  else 
    if (abs(currentWallDistance - lastWallDistance) > 1.0) {
      // We only adjust if we've traveled at least 1/2 our length since the state changed
      if (getDistanceTraveledForTime(getElapsedSinceStartedMoving() - movementState.elapsedStateChange) > (OVERALL_LENGTH_LESS_GRIPPER / 2.0)) {
        distanceMovingForward = adjustDistanceToWall(startWallDistance, currentWallDistance);
      }
    }
  return 0.0;
}
