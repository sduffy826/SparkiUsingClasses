#include "localizationClass.h"

void LocalizationClass::setPose(float x, float y, float angle) {
  pose.xPos = x;
  pose.yPos = y;
  pose.angle = angle;
}
Pose LocalizationClass::getPose() {
  return pose;
}

// This will convert an angle from one coordinate system to another, i.e. from
// compass system to cartesian or vica versa.
int LocalizationClass::convertCoordinateSystemAngle(int angle) {
  return (450-angle)%360;
}

void LocalizationClass::setCurrentAngle(float angle) {
  pose.angle = angle;
}

float LocalizationClass::getCurrentAngle() {
  return pose.angle;
}

void LocalizationClass::setCurrentXPosition(float x) {
  pose.xPos = x;
}

float LocalizationClass::getCurrentXPosition() {
  return pose.xPos;
}

int LocalizationClass::getCurrentXPositionInMM() {
  return (int)(pose.xPos*10);
}

int LocalizationClass::getCurrentXPositionInCM() {
  return (int)(pose.xPos+.5);
}

void LocalizationClass::setCurrentYPosition(float y) {
  pose.yPos = y;
}

float LocalizationClass::getCurrentYPosition() {
  return pose.yPos;
}

int LocalizationClass::getCurrentYPositionInMM() {
  return (int)(pose.yPos*10);
}

int LocalizationClass::getCurrentYPositionInCM() {
  return (int)(pose.yPos+.5);
}

float LocalizationClass::degreesToRadians(float degrees) {
  return (degrees * (PI / 180.0));
}

float LocalizationClass::radiansToDegrees(float radians) {
  return (radians * (180 / PI));
}

byte LocalizationClass::getQuadrantAngleIsIn(float degrees) {
  return (int)(( ((int)degrees) % 360)/90) + 1;
}

// This is basically mod(angle,360.0), couldn't do it with % and float args so wrote my own
float LocalizationClass::getAngle(float angleInDegrees) {
  if (angleInDegrees >= 360.0)
    return getAngle(angleInDegrees - 360.0);
  else
    if (angleInDegrees < 0.0) 
      return getAngle(angleInDegrees + 360.0);
    else
      return angleInDegrees;
}

// Common function to return our 'real angle'; it's the angleDelta + prior orientation (it's degrees)
float LocalizationClass::calculateRealAngleWithAdjustment(float angleDelta) {
  return getAngle(angleDelta + pose.angle);
}

// Calculate the midpoint between two anglesfloat 
float LocalizationClass::getMidpointBetweenTwoAngles(float angle1, float angle2) {
  return ((angle1-angle2)/2)+angle2;
}


float LocalizationClass::calculateNewX(float distance, float angleDelta) {
  // Angle delta is the angle relative to the last position, the real angle is that
  // value plus the pose.angle (the angle it was at when we started).
  // remember cos is adjacent/hypotenuse
  // xValue is calculated by 
  // .   calculate the x alone (it's cos(angle)*distanceTraveled
  //     add the x alone to the lastRobot x position
  return ((cos(degreesToRadians(calculateRealAngleWithAdjustment(angleDelta))) * distance) + pose.xPos);
}

float LocalizationClass::calculateNewY(float distance, float angleDelta) {
  // Similar to the logic for x, but we use sin here (remember sin is Opposite/Hypotenuse)
  // Angle delta is the angle relative to the last position, the real angle is that
  // value plus the currentAngle (the angle it was at when we started).
  return ((sin(degreesToRadians(calculateRealAngleWithAdjustment(angleDelta))) * distance) + pose.yPos);
}

void LocalizationClass::setNewPosition(float distance, float angleDelta) {
  float newX = calculateNewX(distance, angleDelta);
  float newY = calculateNewY(distance, angleDelta);
  // Set new location
  setPose(newX, newY, calculateRealAngleWithAdjustment(angleDelta));
}

float LocalizationClass::getShortestAngleDeltaToGetToOrientation(float targetOrientation) {
  // This returns the shortest angle to get from currentAngleOrientation to a target orientation
  // The value returned will be + for normal rotation (left/ccw), it'll be negative for 
  // right/ccw rotation.

  // Note I didn't save the delta because I'm trying to preserver memory

  // The delta of targetOrientation - currentAngle is the degrees moving in right rotation (cw)
  if (targetOrientation - getCurrentAngle() < -180.0) {
    // if less than -180.0 then add 360' to it, this will be + angles (moving ccw)
    return (360.0 + (targetOrientation - getCurrentAngle()));
  }
  else if (targetOrientation - getCurrentAngle() > 180.0) {
    // if more than 180' then subtract 360 from it, we'll move in negative direction (cw)
    return -(360 - (targetOrientation - getCurrentAngle()));
  }
  else {
    return (targetOrientation - getCurrentAngle());
  }
}

// Calculate the distance between two poses
float LocalizationClass::distanceBetweenPoses(Pose &firstPose, Pose &secondPose) {
  return sqrt(sq(secondPose.xPos - firstPose.xPos) + sq(secondPose.yPos - firstPose.yPos));
}

// Return a pose that's based on an existing on, and an angle of movement and distance
Pose LocalizationClass::calculatePose(Pose thePos, int angleOfMovement, int distanceMoved) {
  // Will return a pose give an existing pose that's moved an angle and movement; note we don't use the angle in the pose
  // since the movement could be a different angle, yet the pose of that object remains the same.
  Pose rtnPos;
  rtnPos.xPos = thePos.xPos + (distanceMoved * cos(degreesToRadians(angleOfMovement)));
  rtnPos.yPos = thePos.yPos + (distanceMoved * sin(degreesToRadians(angleOfMovement)));
  rtnPos.angle = thePos.angle;
}

// Triangulate a third pose based on two poses and the angle of movement used to get from first pose to the second... I believe I can
// calculate that angle but can do that down the road... the caller has it so may was well use for now.
Pose LocalizationClass::triangulatePoses(Pose firstPose, Pose secondPose, int angleOfMovement) {

  Pose rtnPose;
  rtnPose.xPos = 0.0;
  rtnPose.yPos = 0.0;
  rtnPose.angle = -1.0;
  
  // Calculate the new position after movement
  unsigned int distanceTraveled = distanceBetweenPoses(firstPose, secondPose);

  // For naming... firstDegrees is degrees of 'firstPose' point angle, secondDegrees is similar... these are the
  // angles in the triangle they make up (not their pose angle), the thirdDegrees is stored in rtnPose.angle (save memory :))
  // It's not needed anyway so may as well use it
  unsigned int firstDegrees = abs(angleOfMovement - (int)firstPose.angle);
  rtnPose.angle = abs((int)secondPose.angle - (int)firstPose.angle);
  
  if (rtnPose.angle < 90) {  // Must be an intersection
    unsigned int secondDegrees = 180 - (firstDegrees + rtnPose.angle);

    // Sine law is that oppositeSide/sine(angle) are all the same for any triangle... cool :)
    float sineLawThird = distanceTraveled/sin(degreesToRadians(rtnPose.angle));  

    // We can now derive the distance of line opposite first angle (that and secondPose gives use point we want)
    float distanceToLight = sineLawThird * sin(degreesToRadians(firstDegrees));
    rtnPose.xPos = secondPose.xPos + (distanceToLight * cos(degreesToRadians(secondDegrees)));
    rtnPose.yPos = secondPose.yPos + (distanceToLight * sin(degreesToRadians(secondDegrees)));
  }
  

  /* Old way
  unsigned int firstLightAngle  = abs(angleOfMovement - (int)firstPose.angle);
  unsigned int secondLightDeltaAngle = abs((int)secondPose.angle - (int)firstPose.angle);
  if (secondLightDeltaAngle < 90) {
    // Continue... if it's >= 90 then error
    unsigned int secondLightAngle = 90 - secondLightDeltaAngle;
    unsigned int thirdLightAngle = 180 - (firstLightAngle + secondLightAngle);
    float sineLawThird = distanceTraveled/sin(degreesToRadians(thirdLightAngle));
    float distanceToLight = sineLawThird * sin(degreesToRadians(firstLightAngle));
    rtnPose.xPos = secondPos.xPos + (distanceToLight * cos(degreesToRadians(secondLightAngle)));
    rtnPose.yPos = secondPos.yPos + (distanceToLight * sin(degreesToRadians(secondLightAngle)));
    rtnPose.angle = thirdLightAngle; // not needed but give anyway
  } */
  
  /* Pose movedPose = calculatePose(firstPose, 
struct Pose {
  float xPos;
  float yPos;
  float angle;
};
*/
    
}


 
// For debugging we may want to show values on lcd screen
void LocalizationClass::showLocation() {
   #if USE_LCD 
    sparki.clearLCD(); // wipe the LCD clear
    sparki.print("x: ");
    sparki.print(pose.xPos);
    sparki.print(" y: ");
    sparki.print(pose.yPos);
    sparki.print(" angle: ");
    sparki.println(pose.angle);
    sparki.updateLCD(); // put the drawings on the screen
  #else
    Serial.print("LO,x,");
    Serial.print(pose.xPos);
    Serial.print(",y,");
    Serial.print(pose.yPos);
    Serial.print(",<,");
    Serial.println(pose.angle);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}
