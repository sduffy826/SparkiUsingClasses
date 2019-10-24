#include "determineWorldClass.h"

DetermineWorldClass::DetermineWorldClass() { }  // Default constructor, shouldn't be used but needed for compilation on classes that take this as an argument in their constructor

DetermineWorldClass::DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj) {
    ultrasonicObject = ultrasonicObj;
    localizationObject = localizationObj;
    worldXDimension = 0.0;
    worldYDimension = 0.0;
}

void  DetermineWorldClass::adjustWorldCoordinate(float newX, float newY) {
  worldXDimension = newX;
  worldYDimension = newY;
}

float DetermineWorldClass::getWorldXDimension() {
  return worldXDimension;
}

float DetermineWorldClass::getWorldYDimension() {
  return worldYDimension;
}

void DetermineWorldClass::showWorld() {
  // Write to the lcd or the serial device 
  #if USE_LCD 
    sparki.print("World dimensions x: ");
    sparki.print(worldXDimension);
    sparki.print(" y: ");
    sparki.println(worldYDimension);
    
    sparki.print("My position x: ");
    sparki.print(localizationObject.getCurrentXPosition());
    sparki.print(" y: ");
    sparki.println(localizationObject.getCurrentYPosition());
    sparki.updateLCD();
    delay(5000);
  #else
    Serial.print("DW,x,");
    Serial.print(getWorldXDimension());
    Serial.print(",y,");
    Serial.println(getWorldYDimension());

    Serial.print("DP,x,");
    Serial.print(localizationObject.getCurrentXPosition());
    Serial.print(",y,");
    Serial.println(localizationObject.getCurrentYPosition());
    
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
  
  worldXDimension = ultrasonicObject.distanceAtAngle(0) + ULTRASONIC_FORWARD_OF_CENTER;
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);

  // We save the distance from the right wall as the yPosition
  localizationObject.setCurrentYPosition(ultrasonicObject.distanceAtAngle(0) + ULTRASONIC_FORWARD_OF_CENTER); 
  
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);

  localizationObject.setCurrentXPosition(ultrasonicObject.distanceAtAngle(0) + ULTRASONIC_FORWARD_OF_CENTER);
  worldXDimension = worldXDimension + localizationObject.getCurrentXPosition();
  sparki.moveRight(90);
  delay(DELAY_AFTER_MOVEMENT);
  
  worldYDimension = ultrasonicObject.distanceAtAngle(0) + ULTRASONIC_FORWARD_OF_CENTER + localizationObject.getCurrentYPosition();

  // Go back to starting position
  sparki.moveRight(90);
  showWorld();
  localizationObject.setCurrentAngle(0.0);  // We are back at origin, set angle to 0'
}
