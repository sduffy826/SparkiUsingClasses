
#include "ultrasonicClass.h"

UltrasonicClass::UltrasonicClass() {
  this->ultrasonicAngle = 190;
  positionServo(0); 
}

void UltrasonicClass::positionServo(int theAngle) {
  theAngle = (USE_CARTESIANSYSTEM ? -theAngle : theAngle);
  if (ultrasonicAngle != theAngle ) {
    sparki.servo(theAngle + ULTRASONIC_SERVO_ANGLE_ADJUSTMENT);
    
    delay(ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT);
    if ((theAngle - ultrasonicAngle) < -90 ||
        (theAngle - ultrasonicAngle) > 90) {
      delay(ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT);
    }
    ultrasonicAngle = theAngle;
  }
}

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

// To the right is positive degrees and left is negative (not sure why they coded this way)
int UltrasonicClass::distanceRight() {
  return distanceAtAngle(ULTRASONIC_RIGHT_ANGLE);
}

int UltrasonicClass::distanceLeft() {
  return distanceAtAngle(ULTRASONIC_LEFT_ANGLE);
}

float UltrasonicClass::actualBodyDistanceFromFront(const int &reportedDistance) {
  return (float)reportedDistance - 2.0;
}

float UltrasonicClass::actualGripperDistanceFromFront(const int &reportedDistance) {
  return (float)reportedDistance - (2.0 + GRIPPER_LENGTH);
}

float UltrasonicClass::actualCenterOfBodyDistanceFromFront(const int &reportedDistance) {
  return (actualBodyDistanceFromFront(reportedDistance) + (OVERALL_LENGTH_LESS_GRIPPER / 2.0));
}

float UltrasonicClass::actualBodyDistanceFromSide(const int &reportedSideDistance) {
  return (float)reportedSideDistance - 1.825;
}

// This method should only be used when you want to take a reading and calculate what your distance to the wall
// would be at your new orientation... this is mainly used when turning 90' and want to proceed thru an
// opening (you can't read wall distance cause your past it).
float UltrasonicClass::adjustedUltrasonicReadingAfterRotation(const int &reportedDistance, const bool &nowAtZeroAngle) {
  return (float)(nowAtZeroAngle ? (reportedDistance - ULTRASONIC_SIDE_ADJUSTMENT) : (reportedDistance + ULTRASONIC_SIDE_ADJUSTMENT));
}

// For debugging we may want to show values on lcd screen
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
