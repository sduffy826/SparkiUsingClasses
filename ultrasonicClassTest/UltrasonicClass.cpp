
#include "ultrasonicClass.h"

// NOTE: The ultrasonic sensor is with +/- the constant ULTRASONIC_TOLERANCE (.3175 cm at time of this writing (1/8 inch))

// Constructor
UltrasonicClass::UltrasonicClass() {
  this->ultrasonicAngle = 190;
  positionServo(0); 
}

// Position the servo to the angle requested
void UltrasonicClass::positionServo(int theAngle) {
  theAngle = (USE_CARTESIANSYSTEM ? -theAngle : theAngle);
  if (ultrasonicAngle != theAngle ) {
    sparki.servo(theAngle + ULTRASONIC_SERVO_ANGLE_ADJUSTMENT);
    
    delay(ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT);
    // Moving more than 90 degrees delay an additional amount
    if ((theAngle - ultrasonicAngle) < -90 ||
        (theAngle - ultrasonicAngle) > 90) {
      delay(ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT);
    }
    ultrasonicAngle = theAngle;
  }
}

// Get distance at a particular angle
int UltrasonicClass::distanceAtAngle(const int &angleOfServo) {
  positionServo(angleOfServo);
  ultrasonicTempInt = 0;
  for (byte i = 0; i < ULTRASONIC_SAMPLE_SIZE; i++) {
    delay(ULTRASONIC_SAMPLE_DELAY);
    ultrasonicTempInt += sparki.ping();
  }
  ultrasonicTempInt = (int)( ((float)ultrasonicTempInt / (float)ULTRASONIC_SAMPLE_SIZE) + .5);
  return ultrasonicTempInt > ULTRASONIC_MAX_RANGE ? ULTRASONIC_MAX_RANGE : ultrasonicTempInt;
}

// Get distance at right angle
int UltrasonicClass::distanceRight() {
  return distanceAtAngle(ULTRASONIC_RIGHT_ANGLE);
}

// Get distance at left angle
int UltrasonicClass::distanceLeft() {
  return distanceAtAngle(ULTRASONIC_LEFT_ANGLE);
}

// Get the actual open space between front of robot and the object it reported
float UltrasonicClass::getActualBodyDistanceFromFront(const int &reportedDistance) {
  return (float)reportedDistance - 2.0;
}

// Get distance between gripper tip and the object.
float UltrasonicClass::getActualGripperDistanceFromFront(const int &reportedDistance) {
  return (float)reportedDistance - (2.0 + GRIPPER_LENGTH);
}

// Get distance between the center of robot and the object it reported in front of it
float UltrasonicClass::getActualCenterOfBodyDistanceFromFront(const int &reportedDistance) {
  return (getActualBodyDistanceFromFront(reportedDistance) + (OVERALL_LENGTH_LESS_GRIPPER / 2.0));
}

// Get the distance between side of robot and the wall it's next to 
float UltrasonicClass::getActualBodyDistanceFromSide(const int &reportedSideDistance) {
  return (float)reportedSideDistance - 1.825;
}

// Get distance between the center of robot and the object it reported on it's side.
float UltrasonicClass::getActualCenterOfBodyDistanceFromSide(const int &reportedSideDistance) {
  return (getActualBodyDistanceFromSide(reportedSideDistance) + (OVERALL_WIDTH / 2.0));
}

// This method should only be used when you want to take a reading and calculate what your distance to the wall
// would be at your new orientation... this is mainly used when turning 90' and want to proceed thru an
// opening (you can't read wall distance cause your past it).
float UltrasonicClass::getAdjustedUltrasonicReadingAfterRotation(const int &reportedDistance, const bool &nowAtZeroAngle) {
  return (float)(nowAtZeroAngle ? (reportedDistance - ULTRASONIC_SIDE_ADJUSTMENT) : (reportedDistance + ULTRASONIC_SIDE_ADJUSTMENT));
}

// If want to output values
void UltrasonicClass::showUltrasonic(const int &theAngle, const int &theDistance) {
   #if USE_LCD 
    sparki.clearLCD(); // wipe the LCD clear
    sparki.print("<: ");
    sparki.print(theAngle);
    sparki.print("distance: ");
    sparki.println(theDistance);
    sparki.updateLCD(); // put the drawings on the screen
  #else
    Serial.print("US,<,");
    Serial.print(theAngle);
    Serial.print(",d,");
    Serial.println(theDistance);
    delay(DELAY_FOR_SERIAL_COMM);
  #endif
}
