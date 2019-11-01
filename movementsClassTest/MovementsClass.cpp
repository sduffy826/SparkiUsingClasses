#include "movementsClass.h"

// MINWALLOPENINGDEPT is
// actual distance we are from the wall + 1/2 our robot width (this brings us to the all)  (this is about 9 cm)
// + robotLength + gripper length   (this is 14 cm)
// + little fluff  (2cm)  that's how I came up with 25
// Have method for this when u get time... also the same for the minwallwidth opening,
//  that should be width of robot + some fluff :)
#define MINWALLOPENINGDEPTH 25.0  // put in good values later
#define MINWALLOPENINGWIDTH 15.5
#define DEBUGWALL true

MovementsClass::MovementsClass() { }

// Constructor pass in the objects needed 
MovementsClass::MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject) {
  ultrasonicObj = &ultrasonicObject;
  localizationObj = &localizationObject;
  determineWorldObj = &determineWorldObject;
  initMovements();
}


void MovementsClass::initMovements() {
  ultrasonicObj->positionServo(0);
  movementState.amMoving  = false;
  movementState.wallOpening = false;  // indicator for when there's an opening in the wall
  movementState.rightWall = true;  // Means want to follow wall with wall on the right
  
  movementState.startTime = 0;
  movementState.elapsedStateChange = 0;
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
unsigned int MovementsClass::getMillisToGetThere(const float &distanceInCM) {
  return (distanceInCM / VELOCITY_CM_P_SEC) * 1000;
}

// Routine to signify we're moving
void MovementsClass::startMoving(const bool &goForward) {
  if ( movementState.amMoving == false ) {
    movementState.amMoving = true;
    movementState.startTime = millis();
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

// Return distance travelled for duration
float MovementsClass::getDistanceTraveledForTime(const int &milliseconds) {
  return ((milliseconds * VELOCITY_CM_P_SEC) / 1000.0);
}

// Return the distance traveled so far
float MovementsClass::getDistanceTraveledSoFar() {
  if ( movementState.amMoving == true ) {
    return getDistanceTraveledForTime(getElapsed());
  }
  return 0.0;
}

// Stop moving and return the distance traveled
void MovementsClass::stopMoving() {
  // Set your position, the angle delta is 0 since we're going in a straight line
  if (movementState.inReverse) 
    localizationObj->setNewPosition(-getDistanceTraveledSoFar(), 0.0);
  else
    localizationObj->setNewPosition(getDistanceTraveledSoFar(), 0.0);
  sparki.moveStop();
  movementState.amMoving = false;
  determineWorldObj->checkWorldCoordinates(); // We may have found new world positions, check em
}

// Handles moving backward a given distance, second parm is the min distance you can be within of a wall
// Intent is that this will be called continuously... it doesn't block... it'll start moving if sparki isn't
// and it'll stop when we've reached destination or wall.
boolean MovementsClass::moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance) {
  if ( movementState.amMoving == true ) {
    if (getDistanceTraveledSoFar() >= distanceToTravel) {  // done moving
      stopMoving();
    }
    else {
      if (checkFrontDistance && (ultrasonicObj->distanceAtAngle(0) >= minAllowedDistanceToObstacle)) {  // done moving
        stopMoving();
      }
    }
  }
  else {
    if ((getMillisToGetThere(distanceToTravel) <= 0.0) || (checkFrontDistance && (ultrasonicObj->distanceAtAngle(0) >= minAllowedDistanceToObstacle))) {  // cant move
      sparki.beep();
    }
    else {
      startMoving(false);  // False means go in reverse
    }
  } 
  return movementState.amMoving;
}  


// Similar to above but moves foward.
boolean MovementsClass::moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance) {
  if ( movementState.amMoving == true ) {
    if (getDistanceTraveledSoFar() >= distanceToTravel) {  // done moving
      stopMoving();
    }
    else {
      if (checkFrontDistance && (ultrasonicObj->distanceAtAngle(0) <= minAllowedDistanceToObstacle)) {  // done moving
        stopMoving();
      }
    }
  }
  else {
    if ((getMillisToGetThere(distanceToTravel) <= 0.0) || (checkFrontDistance && ultrasonicObj->distanceAtAngle(0) <= minAllowedDistanceToObstacle)) {  // cant move
      sparki.beep();
    }
    else {
      startMoving(true);
    }
  } 
  return movementState.amMoving;
}  

// Turn left or right byt certain degrees and set your current orientation angle
void MovementsClass::turnLeft(const int &degrees) {
  sparki.moveLeft(degrees);
  localizationObj->setCurrentAngle(localizationObj->calculateRealAngleWithAdjustment(-degrees));
}
  
void MovementsClass::turnRight(const int &degrees) {
  sparki.moveRight(degrees);
  localizationObj->setCurrentAngle(localizationObj->calculateRealAngleWithAdjustment(degrees));
}

int MovementsClass::getClosest90Angle() {
  return (( ((localizationObj->getCurrentAngle()+44)/90) * 90) % 360);
}

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

// Turn to specific angle and return distance in front of you :)
int MovementsClass::getDistanceAtAngle(const int &angle) {
  turnToAngle(angle);
  return ultrasonicObj->distanceAtAngle(0);
}

// Convenience routine
void MovementsClass::turnToZero() {
  turnToAngle(0);
}

// Another convenience routine
void MovementsClass::turnTo90ClosestDegreeOrientation() {
  turnToAngle(getClosest90Angle());
}

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

// This SHOULD not be called if we're on a wall opening, cause values get reset after stopping
int MovementsClass::adjustDistanceToWall(const int &desiredDistance, const int &currentWallDistance) {
  stopMoving();
  turnRight(90);  // turnLogic
  if (desiredDistance > currentWallDistance) {  // Too close backup
    while (moveBackward(desiredDistance-currentWallDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  else { 
    while (moveForward(currentWallDistance-desiredDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  turnLeft(90);
  int newDistanceForward = ultrasonicObj->distanceAtAngle(0);
  
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
//   updateDistanceOfWallOpening
//   moveState.distanceOfOpening += distanceTraveledSoFar;
//   if moveState.distanceOfOpening >= WALLOPENINGTOREPORT {
//     stop
//     goThruOpening
float MovementsClass::wallOpeningDistance(int &distanceMovingForward, const int &startWallDistance, const int &lastWallDistance) {

  int currentWallDistance = ultrasonicObj->distanceAtAngle(90);  // turnLogic

  if (currentWallDistance > MINWALLOPENINGDEPTH) {
    if (movementState.wallOpening == false) {
      movementState.wallOpening = true;
      movementState.elapsedStateChange = getElapsed();
    
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
      movementState.elapsedStateChange = getElapsed();
    }
  }
  
  if (movementState.wallOpening) {
    // if at wall opening see if we should handle it... i.e turn down it
    return (getDistanceTraveledForTime(getElapsed() - movementState.elapsedStateChange));
  }
  else 
    if (abs(currentWallDistance - lastWallDistance) > 1) {
      // We only adjust if we've traveled at least 1/2 our length since the state changed
      if (getDistanceTraveledForTime(getElapsed() - movementState.elapsedStateChange) > (OVERALL_LENGTH_LESS_GRIPPER / 2.0)) {
        distanceMovingForward = adjustDistanceToWall(startWallDistance, currentWallDistance);
      }
    }
  return 0.0;
}


// -------------------------- 
// Right now it's following the right wall, if want left wall then change the 'turnRight' to left and 
// vica versa... I marked commend on lines neededing change (look at 'turnLogic' comment).
// it smart enough to turn right too
void MovementsClass::followWall() {
  int saveX, saveY;
  int theDistance;
  int startWallDistance, lastWallDistance, widthOfOpening;
  byte numTurns;
  // Face the wall to the right
  turnRight(90);  // turnLogic
  theDistance = ultrasonicObj->distanceAtAngle(0);

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
    theDistance = ultrasonicObj->distanceAtAngle(0);
    startWallDistance = lastWallDistance = ultrasonicObj->distanceAtAngle(90);    // turnLogic
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
        theDistance = ultrasonicObj->distanceAtAngle(0);
      }
      // had delay(100) but removed it... see how it works.
    }
  } 
}

// Crude, put in logic for obstacles
void MovementsClass::moveToPose(const Pose &targetPose) {
  
  Pose currentPose = localizationObj->getPose();
  
  float targetAngle = localizationObj->calculateAngleBetweenPoints(currentPose.xPos, currentPose.yPos, targetPose.xPos, targetPose.yPos);
  
  // Turn to specific angle and return distance in front of you :)
  int openSpace = getDistanceAtAngle(targetAngle);
  float distance2Move = localizationObj->distanceBetweenPoses(currentPose, targetPose);
  if ((float)openSpace < distance2Move) {
    distance2Move = (float)openSpace;
  }
  while (moveForward(distance2Move, ULTRASONIC_MIN_SAFE_DISTANCE, true));
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
