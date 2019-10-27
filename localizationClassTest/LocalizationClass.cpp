#include "localizationClass.h"

void LocalizationClass::setPose(const float &x, const float &y, const int &angle) {
  pose.xPos = x;
  pose.yPos = y;
  pose.angle = angle;
}
Pose LocalizationClass::getPose() {
  return pose;
}

// This will convert an angle from one coordinate system to another, i.e. from
// compass system to cartesian or vica versa.
int LocalizationClass::convertCoordinateSystemAngle(const int &angle) {
  return (450-angle)%360;
}

void LocalizationClass::setCurrentAngle(const int &angle) {
  pose.angle = angle;
}

int LocalizationClass::getCurrentAngle() {
  return pose.angle;
}

void LocalizationClass::setCurrentXPosition(const float &x) {
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

void LocalizationClass::setCurrentYPosition(const float &y) {
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

float LocalizationClass::degreesToRadians(const int &degrees) {
  return (degrees * (PI / 180.0));
}

int LocalizationClass::radiansToDegrees(const float &radians) {
  return (int)(radians * (180 / PI));
}

byte LocalizationClass::getQuadrantAngleIsIn(const int &degrees) {
  return (getAngle(degrees) / 90) + 1;
}

// This is basically mod(angle,360.0), couldn't do it with % and float args so wrote my own
int LocalizationClass::getAngle(int angleInDegrees) {
  if (angleInDegrees >= 360)
    return getAngle(angleInDegrees - 360);
  else
    if (angleInDegrees < 0) 
      return getAngle(angleInDegrees + 360);
    else
      return angleInDegrees;
}

// Common function to return our 'real angle'; it's the angleDelta + prior orientation (it's degrees)
int LocalizationClass::calculateRealAngleWithAdjustment(const int &angleDelta) {
  return getAngle(angleDelta + pose.angle);
}

// Calculate the midpoint between two angles
int LocalizationClass::getMidpointBetweenTwoAngles(const int &angle1, const int &angle2) {
  return ((angle1-angle2)/2)+angle2;
}

// This calculates the new position based on distance and angle 
float LocalizationClass::calculateNewX(const float &distance, const int &angleDelta) {
  // Angle delta is the angle relative to the last position, the real angle is that
  // value plus the pose.angle (the angle it was at when we started).
  // remember cos is adjacent/hypotenuse
  // xValue is calculated by 
  // .   calculate the x alone (it's cos(angle)*distanceTraveled
  //     add the x alone to the lastRobot x position
  return ((cos(degreesToRadians(calculateRealAngleWithAdjustment(angleDelta))) * distance) + pose.xPos);
}

float LocalizationClass::calculateNewY(const float &distance, const int &angleDelta) {
  // Similar to the logic for x, but we use sin here (remember sin is Opposite/Hypotenuse)
  // Angle delta is the angle relative to the last position, the real angle is that
  // value plus the currentAngle (the angle it was at when we started).
  return ((sin(degreesToRadians(calculateRealAngleWithAdjustment(angleDelta))) * distance) + pose.yPos);
}

void LocalizationClass::setNewPosition(const float &distance, const int &angleDelta) {
  float newX = calculateNewX(distance, angleDelta);
  float newY = calculateNewY(distance, angleDelta);
  // Set new location
  setPose(newX, newY, calculateRealAngleWithAdjustment(angleDelta));
}

int LocalizationClass::getShortestAngleDeltaToGetToOrientation(const int &targetOrientation) {
  // This returns the shortest angle to get from currentAngleOrientation to a target orientation
  // The value returned will be + for normal rotation (left/ccw), it'll be negative for 
  // right/ccw rotation.

  // Note I didn't save the delta because I'm trying to preserver memory

  // The delta of targetOrientation - currentAngle is the degrees moving in right rotation (cw)
  if (targetOrientation - getCurrentAngle() < -180) {
    // if less than -180.0 then add 360' to it, this will be + angles
    return (360 + (targetOrientation - getCurrentAngle()));
  }
  else if (targetOrientation - getCurrentAngle() > 180) {
    // if more than 180' then subtract 360 from it, we'll move in negative direction (ccw)
    return -(360 - (targetOrientation - getCurrentAngle()));
  }
  else {
    // Return difference, if target larger than current it'll be a positive value (cw) and vica versa if neg :)
    return (targetOrientation - getCurrentAngle()); 
  }
}

// Calculate the distance between two poses
float LocalizationClass::distanceBetweenPoses(const Pose &firstPose, const Pose &secondPose) {
  return sqrt(sq(secondPose.xPos - firstPose.xPos) + sq(secondPose.yPos - firstPose.yPos));
}

// Return a pose that's based on an existing on, and an angle of movement and distance
Pose LocalizationClass::calculatePose(const Pose &thePos, const int &angleOfMovement, const int &distanceMoved) {
  // Will return a pose give an existing pose that's moved an angle and movement; note we don't use the angle in the pose
  // since the movement could be a different angle, yet the pose of that object remains the same.
  Pose rtnPos;
  rtnPos.xPos = thePos.xPos + (distanceMoved * cos(degreesToRadians(angleOfMovement)));
  rtnPos.yPos = thePos.yPos + (distanceMoved * sin(degreesToRadians(angleOfMovement)));
  rtnPos.angle = thePos.angle;
  return rtnPos;
}

// Triangulate a third pose based on two poses and the angle of movement used to get from first pose to the second... I believe I can
// calculate that angle but can do that down the road... the caller has it so may was well use for now.

// The pose returned has the x and y position of the triangulated object; 
Pose LocalizationClass::triangulatePoses(const Pose &firstPose, const Pose &secondPose, const int &angleOfMovement) {

  Pose rtnPose;
  rtnPose.xPos = 0.0;
  rtnPose.yPos = 0.0;
  rtnPose.angle = -1;
  
  // Calculate the new position after movement
  float distanceTraveled = distanceBetweenPoses(firstPose, secondPose);

  // For naming... firstDegrees is degrees of 'firstPose' point angle, secondDegrees is similar... these are the
  // angles in the triangle they make up (not their pose angle), the thirdDegrees is stored in rtnPose.angle (save memory :))
  // It's not needed anyway so may as well use it
  unsigned int firstDegrees = abs(angleOfMovement - firstPose.angle);
  rtnPose.angle = abs(secondPose.angle - firstPose.angle);
  
  if (rtnPose.angle < 90) {  // Must be an intersection which means less than 90'
    unsigned int secondDegrees = 180 - (firstDegrees + rtnPose.angle);

    // Sine law is that oppositeSide/sine(angle) are all the same for any triangle... cool :)
    float sineLawThird = distanceTraveled/sin(degreesToRadians(rtnPose.angle));  

    // We can now derive the distance of line opposite first angle (that and secondPose gives use point we want)
    float distanceToLight = sineLawThird * sin(degreesToRadians(firstDegrees));

    rtnPose = calculatePose(secondPose, secondDegrees, distanceToLight);
    // The angle returned from method above is secondPose's angle, put it back
    rtnPose.angle = abs(secondPose.angle - firstPose.angle);
    
    //rtnPose.xPos = secondPose.xPos + (distanceToLight * cos(degreesToRadians(secondDegrees)));
    //rtnPose.yPos = secondPose.yPos + (distanceToLight * sin(degreesToRadians(secondDegrees)));
  }
  return rtnPose;

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
void LocalizationClass::showPose(const Pose &pos2Show) {
   #if USE_LCD 
    sparki.clearLCD(); // wipe the LCD clear
    sparki.print("x: ");
    sparki.print(pos2Show.xPos);
    sparki.print(" y: ");
    sparki.print(pos2Show.yPos);
    sparki.print(" angle: ");
    sparki.println(pos2Show.angle);
    sparki.updateLCD(); // put the drawings on the screen
  #else
    Serial.print("PO,x,");
    Serial.print(pos2Show.xPos);
    Serial.print(",y,");
    Serial.print(pos2Show.yPos);
    Serial.print(",<,");
    Serial.println(pos2Show.angle);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}

 
// Call routine to show my pose
void LocalizationClass::showLocation() {
   showPose(pose);
}
 void LocalizationClass::writeMsg2Serial(char *msg) {
   Serial.println(msg);
   delay(DELAY_FOR_SERIAL_COMM);
 }
