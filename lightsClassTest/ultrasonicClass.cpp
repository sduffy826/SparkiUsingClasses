
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

// Get the angle the servo is at
int UltrasonicClass::getServoAngle() {
  return ultrasonicAngle;
}

// Get distance at a particular angle
float UltrasonicClass::getDistanceFromServoAtAngle(const int &angleOfServo) {
  positionServo(angleOfServo);
  float sumPings = 0;
  for (byte i = 0; i < ULTRASONIC_SAMPLE_SIZE; i++) {
    delay(ULTRASONIC_SAMPLE_DELAY);
    sumPings += sparki.ping();
  }
  sumPings = (sumPings / (float)ULTRASONIC_SAMPLE_SIZE );
  return sumPings > ULTRASONIC_MAX_RANGE ? ULTRASONIC_MAX_RANGE : sumPings;
}

// Return the distance from the center of the robot to the obstacle
float UltrasonicClass::getDistanceFromCenterOfRobotToObstacle(const int &angleOfServo) {
  // The center of the robot is ULTRASONIC_SERVO_TO_PIVOT + ULTRASONIC_SERVO_PIVOT_OFFSET when
  // it's at angle 0.. as it moves toward 90 the PIVOT_OFFSET is reduced, at 90' it's zero, that's
  // why we have the formula below
  return ( getDistanceFromServoAtAngle(angleOfServo) + ULTRASONIC_SERVO_TO_PIVOT + ( ((90 - abs(angleOfServo))/90.0) * ULTRASONIC_SERVO_PIVOT_OFFSET) );
}

// Return the distance from the center of the robot to the obstacle
float UltrasonicClass::getFreeSpaceInFrontExcludingGripper(const int &angleOfServo) {
  return (getDistanceFromCenterOfRobotToObstacle(angleOfServo) - (OVERALL_LENGTH_LESS_GRIPPER / 2.0));
}

float UltrasonicClass::getFreeSpaceInFrontOfGripper(const int &angleOfServo) {
  return (getFreeSpaceInFrontExcludingGripper(angleOfServo) - GRIPPER_LENGTH);
}

// Get distance at right angle
float UltrasonicClass::getFreeSpaceOnRight() {
  return (getDistanceFromCenterOfRobotToObstacle(ULTRASONIC_RIGHT_ANGLE) - (OVERALL_WIDTH / 2.0));
}

// Get distance at left angle
float UltrasonicClass::getFreeSpaceOnLeft() {
  return (getDistanceFromCenterOfRobotToObstacle(ULTRASONIC_LEFT_ANGLE) - (OVERALL_WIDTH / 2.0));
}

float UltrasonicClass::getSensorTolerance() {
  return (ULTRASONIC_TOLERANCE);
}

// If want to output values just pass them in... I don't do anything with servo in here, it's just a helper method
void UltrasonicClass::showUltrasonic(const int &theAngle, const float &theDistance) {
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
