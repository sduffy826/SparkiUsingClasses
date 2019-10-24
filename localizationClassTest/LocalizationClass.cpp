#include "localizationClass.h"

void LocalizationClass::setPose(float x, float y, float angle) {
  pose.xPos = x;
  pose.yPos = y;
  pose.angle = angle;
}
Pose LocalizationClass::getPose() {
  return pose;
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