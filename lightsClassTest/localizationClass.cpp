#include "localizationClass.h"
#define DEBUGTRIAN false 

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

// Little helper to determine if two float values are within an allowable range 
boolean LocalizationClass::closeEnuf(const float &value1, const float &value2, const float &allowedDelta, const boolean &areAngles) {
  if (areAngles == true) {
     // Call myself with angles converted and say we're not doing angles
     return closeEnuf(getAngle(value1),getAngle(value2),allowedDelta,false); 
  }
  else {
    if ((value1 - value2) > allowedDelta) {
      return false;  
    }
    else
      if ((value1 - value2) < -allowedDelta) {
        return false;
      }
    return true;
  }
}

int LocalizationClass::calculateAngleBetweenPoints(const float &x1, const float &y1, const float &x2, const float &y2) {
  #if DEBUGTRIAN
    Serial.print("< between points ");
    Serial.print(" 1: (");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.print(") 2: (");
    Serial.print(x2);
    Serial.print(",");
    Serial.print(y2);
    Serial.print(") atan2(): ");
    Serial.print(atan2(y2-y1,x2-x1));
    Serial.print(" * (180/PI) ");
    Serial.println(atan2(y2-y1,x2-x1)*(180.0/PI));
  #endif
   
  return atan2(y2-y1,x2-x1)*(180.0/PI);
}

// Get slop of pose
float LocalizationClass::getSlopeOfAngle(const int &theAngle) {
  if ((theAngle % 180) == 0) {  
    return 0;  // Slope is 0 at 0 and 180'
  }
  else 
    if ((theAngle % 90) == 0) {
      // It's undefined (infinity) at 90 adn 270, we'll return 100 (a big #)
      return 100;
    }
    else {  
      // The slope is change change in y/x (which is tangent)
      return tan(degreesToRadians(theAngle));
    }
}

// Return the y intercept for the pose
float LocalizationClass::getYInterceptForPose(const Pose &thePose) {
  return (thePose.yPos - (getSlopeOfAngle(thePose.angle) * thePose.xPos));
}

// Sets point of intersection between two poses
boolean LocalizationClass::setPointOfIntersection(const Pose &pose1, const Pose &pose2, Pose &pose2Update) {
  pose2Update.xPos = 0.0;
  pose2Update.yPos = 0.0;
  pose2Update.angle = -1; // Invalid value

  // Use variable to save slopes
  pose2Update.xPos = getSlopeOfAngle(pose1.angle);
  pose2Update.yPos = getSlopeOfAngle(pose2.angle);
  #if DEBUGTRIAN
    Serial.print("Slopes");
    Serial.print(" 1: ");
    Serial.print(pose2Update.xPos);
    Serial.print(" 2:");
    Serial.println(pose2Update.yPos);
  #endif
  
  if (pose2Update.xPos == pose2Update.yPos) {
    return false; // Undefined, slopes are the same...  caller will have to inspect angles
  }
  if (pose2Update.xPos == 0.0) {
    // We will call routine again flipping first and second positions... we want the first one to have a slope
    // for the formulas that follow
    return setPointOfIntersection(pose2, pose1, pose2Update);
  }

  // Below is basically (pose2 intercept - pose1 intercept) / (pose1 slope - pose2 slope) 
  // YES order above is correct... the formula's are setting y=mx+b, setting formulas equal and
  // solving for x
  pose2Update.xPos = (getYInterceptForPose(pose2) - getYInterceptForPose(pose1)) / (pose2Update.xPos - pose2Update.yPos);
  #if DEBUGTRIAN
    Serial.print("Y Intercept");
    Serial.print(" 1: ");
    Serial.print(getYInterceptForPose(pose1));
    Serial.print(" 2:");
    Serial.println(getYInterceptForPose(pose2));
  #endif

  // Now solve for y using x value got above... it's basically y-mx+b on the first pose (which we know doens't have slope of 0)
  pose2Update.yPos = getSlopeOfAngle(pose1.angle) * pose2Update.xPos + getYInterceptForPose(pose1);

  #if DEBUGTRIAN
    Serial.println("Poses");
    showPose(pose1);
    showPose(pose2);
    Serial.print("Intersect: ");
    Serial.print(pose2Update.xPos);
    Serial.print(",");
    Serial.print(pose2Update.yPos);
    Serial.print(" < to 1st: ");
    Serial.print(calculateAngleBetweenPoints(pose1.xPos, pose1.yPos, pose2Update.xPos, pose2Update.yPos));
    Serial.print(" < to 2nd: ");
    Serial.println(calculateAngleBetweenPoints(pose2.xPos, pose2.yPos, pose2Update.xPos, pose2Update.yPos));
  #endif

  // Check that the angles in our pose are the same as angle to get to the point of intersection
  if ( closeEnuf(calculateAngleBetweenPoints(pose1.xPos, pose1.yPos, pose2Update.xPos, pose2Update.yPos),pose1.angle,1.0,true) &&
       closeEnuf(calculateAngleBetweenPoints(pose2.xPos, pose2.yPos, pose2Update.xPos, pose2Update.yPos),pose2.angle,1.0,true) ) {
    pose2Update.angle = 0.0;     
    return true; 
  }
  
  return false;
}


float LocalizationClass::degreesToRadians(const int &degrees) {
  return (degrees * (PI / 180.0));
}

int LocalizationClass::radiansToDegrees(const float &radians) {
  return (int)(radians * (180.0 / PI));
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
/*
Pose LocalizationClass::triangulatePoses(const Pose &firstPose, const Pose &secondPose, const int &angleOfMovement) {

  Pose rtnPose;
  rtnPose.xPos = 0.0;
  rtnPose.yPos = 0.0;
  rtnPose.angle = -1;
  
  // Calculate the new position after movement
  float distanceTraveled = distanceBetweenPoses(firstPose, secondPose);

  #if DEBUGTRIAN
    Serial.print("distTrav: ");
    Serial.println(distanceTraveled);
  #endif
  // For naming... firstDegrees is degrees of 'firstPose' point angle, secondDegrees is similar... these are the
  // angles in the triangle they make up (not their pose angle), the thirdDegrees is stored in rtnPose.angle (save memory :))
  // It's not needed anyway so may as well use it
  unsigned int firstDegrees = abs(angleOfMovement - firstPose.angle);
  rtnPose.angle = abs(secondPose.angle - firstPose.angle);  // Just making sure the pose angles are less than 90 degrees

  #if DEBUGTRIAN
    Serial.print("firstDegrees: ");
    Serial.print(firstDegrees);
    Serial.print(" rtnPose.angle: ");
    Serial.println(rtnPose.angle);
  #endif
  
  if (rtnPose.angle < 90) {  // Must be an intersection which means less than 90'
    // old unsigned int secondDegrees = 180 - (firstDegrees + rtnPose.angle);
    
    // Want interior angle so it's 180-angle 
    unsigned int secondDegrees = 180 - abs(angleOfMovement - secondPose.angle);
    // Third angle
    rtnPose.angle = 180 - (secondDegrees + firstDegrees);
    
    #if DEBUGTRIAN
      Serial.print("secondDegrees: ");
      Serial.print(secondDegrees);
      Serial.print(" thirdDegrees: ");
      Serial.println(rtnPose.angle);
    #endif

    // Sine law is that oppositeSide/sine(angle) are all the same for any triangle... cool :)
    float sineLawThird = distanceTraveled/sin(degreesToRadians(rtnPose.angle));  

    #if DEBUGTRIAN
      Serial.print("sineLawThird: ");
      Serial.println(sineLawThird);
    #endif

    // We can now derive the distance of line opposite first angle (that and secondPose gives use point we want)
    float distanceToLight = sineLawThird * sin(degreesToRadians(firstDegrees));

    #if DEBUGTRIAN
      Serial.print("dist2Light: ");
      Serial.println(distanceToLight);
    #endif

    // old rtnPose = calculatePose(secondPose, secondDegrees, distanceToLight);

    // Calculate the new pose... we're starting at second point, at his angle and the
    // distance to the light... that's the location where light exists
    rtnPose = calculatePose(secondPose, secondPose.angle, distanceToLight);
    
    // The angle returned from method above is secondPose's angle, put it back
    // old rtnPose.angle = abs(secondPose.angle - firstPose.angle);
    rtnPose.angle = 180 - (secondDegrees + firstDegrees);
    
    //rtnPose.xPos = secondPose.xPos + (distanceToLight * cos(degreesToRadians(secondDegrees)));
    //rtnPose.yPos = secondPose.yPos + (distanceToLight * sin(degreesToRadians(secondDegrees)));
  }
  return rtnPose;

  // Old way
  //unsigned int firstLightAngle  = abs(angleOfMovement - (int)firstPose.angle);
  //unsigned int secondLightDeltaAngle = abs((int)secondPose.angle - (int)firstPose.angle);
  //if (secondLightDeltaAngle < 90) {
  //  // Continue... if it's >= 90 then error
  //  unsigned int secondLightAngle = 90 - secondLightDeltaAngle;
  //  unsigned int thirdLightAngle = 180 - (firstLightAngle + secondLightAngle);
  //  float sineLawThird = distanceTraveled/sin(degreesToRadians(thirdLightAngle));
  //  float distanceToLight = sineLawThird * sin(degreesToRadians(firstLightAngle));
  //  rtnPose.xPos = secondPos.xPos + (distanceToLight * cos(degreesToRadians(secondLightAngle)));
  //  rtnPose.yPos = secondPos.yPos + (distanceToLight * sin(degreesToRadians(secondLightAngle)));
  //  rtnPose.angle = thirdLightAngle; // not needed but give anyway
  //}    
} */ // End of block commenting out the triangle method

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
