#include "determineWorldClass.h"

DetermineWorldClass::DetermineWorldClass() { }  // Default constructor, shouldn't be used but needed for compilation on classes that take this as an argument in their constructor

DetermineWorldClass::DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj) {
    ultrasonicObject = &ultrasonicObj;
    localizationObject = &localizationObj;

    worldCoord.xMax = 0.0;
    worldCoord.xMin = 0.0;
    worldCoord.yMax = 0.0;
    worldCoord.yMin = 0.0;


    // Make a callback to the ultrasonicObj to give it the address of 
    ultrasonicObject->setDetermineWorldObj(*this);
    ultrasonicObject->setLocalizationObj(localizationObj);
}

// Just return the struct that has your world coordinates
WorldCoord DetermineWorldClass::getWorldCoordinates() {
  return worldCoord;
}

// This uses the localization objects current position and see's if we need to adjust the world coordinates.
void  DetermineWorldClass::checkWorldCoordinates() {
  if (localizationObject->getCurrentAngle() == 0) {
    float tempVar = localizationObject->getCurrentXPosition();
    worldCoord.xMax = (worldCoord.xMax > tempVar ? worldCoord.xMax : tempVar);
    worldCoord.xMin = (worldCoord.xMin < tempVar ? worldCoord.xMin : tempVar);
  
    tempVar = localizationObject->getCurrentYPosition();
    worldCoord.yMax = (worldCoord.yMax > tempVar ? worldCoord.yMax : tempVar);
    worldCoord.yMin = (worldCoord.yMin < tempVar ? worldCoord.yMin : tempVar);
  }
}

// This method is called when one of our sensors identifies an obstacle, it's given the pose from
// where the reading took place and the distance to the obstacle
void recordObstacleFromPoseToLength(const Pose &servoPivotPose, const float &distanceToObstacle) {
  #if USE_LCD
    sparki.clearLCD();
    Serial.println("In World got obstacle");
    Serial.print("Pose,x: ");
    Serial.print(servoPivotPose.xPos);
    Serial.print(" y: ");
    Serial.print(servoPivotPose.yPos);
    Serial.println("<: ");
    Serial.print(servoPivotPose.angle);
    Serial.print(" dist")
    Serial.println(distanceToObstacle);
    sparki.updateLCD();
    delay(3000);
  #else
    Serial.print("DO,x,");
    Serial.print(servoPivotPose.xPos);
    Serial.print(",y,");
    Serial.print(servoPivotPose.yPos);
    Serial.print(",<,");
    Serial.print(servoPivotPose.angle);
    Serial.print(",d,");
    Serial.println(distanceToObstacle);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}

void DetermineWorldClass::showWorld() {
  // Write to the lcd or the serial device 
  #if USE_LCD 
    sparki.println("World dimensions");
    sparki.print("xMin: ");
    sparki.print(worldCoord.xMin);
    sparki.print(" xMax: ");
    sparki.println(worldCoord.xMax);
    sparki.print("yMin: ");
    sparki.print(worldCoord.yMin);
    sparki.print(" yMax: ");
    sparki.println(worldCoord.yMax);
    
    sparki.print("My position x: ");
    sparki.print(localizationObject->getCurrentXPosition());
    sparki.print(" y: ");
    sparki.println(localizationObject->getCurrentYPosition());
    sparki.updateLCD();
    delay(5000);
  #else
    Serial.print("DW,x1,");
    Serial.print(worldCoord.xMin);
    Serial.print(",x2,");
    Serial.print(worldCoord.xMax);
    Serial.print(",y1,");
    Serial.print(worldCoord.yMin);
    Serial.print(",y2,");
    Serial.println(worldCoord.yMax);

    Serial.print("DP,x,");
    Serial.print(localizationObject->getCurrentXPosition());
    Serial.print(",y,");
    Serial.println(localizationObject->getCurrentYPosition());
    
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}

// This calculates the rectangular coordinates of the world for this to
// work the sparki must be perpenticular to a wall, could make it smart
// and take readings within a 45' arc and position itself at the minimum
// distance, but that's not really a requiement for this.

// Note the world coordinates must be within the ultrasounds range... if it's
// larger than that then the world calculated here will be the range... your
// robot should be smart enough to handle that (i.e. if moving and finds world
// dimensions differ, then make an adjustment).
void DetermineWorldClass::calculateRectangularCoordinates() {
  delay(DELAY_AFTER_MOVEMENT);

  worldCoord.xMin = 0.0;
  worldCoord.yMin = 0.0;
  
  worldCoord.xMax = ultrasonicObject->getDistanceFromCenterOfRobotToObstacle(0);
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);

  // We save the distance from the right wall as the yPosition
  localizationObject->setCurrentYPosition(ultrasonicObject->getDistanceFromCenterOfRobotToObstacle(0)); 
  
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);

  localizationObject->setCurrentXPosition(ultrasonicObject->getDistanceFromCenterOfRobotToObstacle(0));
  worldCoord.xMax += localizationObject->getCurrentXPosition();
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);
  
  worldCoord.yMax = ultrasonicObject->getDistanceFromCenterOfRobotToObstacle(0) + localizationObject->getCurrentYPosition();

  // Go back to starting position
  sparki.moveRight(90);
  localizationObject->setCurrentAngle(0);  // We are back at origin, set angle to 0'
}
