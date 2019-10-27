#include "movementsClass.h"
int counter;

void setup() {
  // put your setup code here, to run once:
  counter = 0;
}

void memTestFunction(const float &aFloat, const float &bFloat) {
  int valueOfFloat = aFloat * bFloat;
}

// We don't track distance with this, put that in later
void wiggleForward(const int &timeInSeconds, const int &leftWheel, const int &rightWheel) {
  for (unsigned int i = 0; i < timeInSeconds; i++) {
    // Left forward at 100% with right going backward 20%
    sparki.motorRotate(MOTOR_LEFT, DIR_CCW, leftWheel);
    sparki.motorRotate(MOTOR_RIGHT, DIR_CCW, rightWheel);
    delay(500);
    sparki.moveStop();
    sparki.motorRotate(MOTOR_LEFT, DIR_CW, rightWheel);
    sparki.motorRotate(MOTOR_RIGHT, DIR_CW, leftWheel);
    delay(500);
    sparki.moveStop();
  }
}

// This SHOULD not be called if we're on a wall opening, cause values get reset after stopping
void adjustDistanceToWall(MovementsClass &movementObj, UltrasonicClass &ultrasonicObj, int desiredDistance) {
  // We will calculate distance it takes to change one CM
  movementObj.turnRight(90);
  // Move forward till within min distance
  while (movementObj.moveForward(100, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  int theDistance = ultrasonicObj.distanceAtAngle(0);
  // Now move slow till we hit the wall... this should force it to be at 90'
  //sparki.moveForward(GRIPPER_LENGTH . ULTRASONIC_FORWARD_OF_CENTER . OVERALL_LENGTH_LESS_GRIPPER

  // Distance to move forward is (length/2-ultrasonic_forward_of_center)+gripper_length
  
  wiggleForward(5,100,50);
  sparki.beep();
  
  theDistance = ultrasonicObj.distanceAtAngle(0);
  while (theDistance < ULTRASONIC_MIN_SAFE_DISTANCE) {
    sparki.moveBackward();
    theDistance = ultrasonicObj.distanceAtAngle(0);
  }
  sparki.moveStop();
  movementObj.turnLeft(90);

/*  
  stopMoving();
  turnRight(90);  // turnLogic
  if (desiredDistance > currentWallDistance) {  // Too close backup
    while (moveBackward(desiredDistance-currentWallDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  else { 
    while (moveForward(currentWallDistance-desiredDistance, ULTRASONIC_MIN_SAFE_DISTANCE, true));
  }
  turnLeft(90);
  newDistanceForwardAfterAdjustment = ultrasonicObj.distanceAtAngle(0);
  
  #if DEBUGWALL
    Serial.print("In adjustDistanceToWall");
    Serial.print(" desired: ");
    Serial.print(desiredDistance);
    Serial.print(" current: ");
    Serial.print(currentWallDistance);
    Serial.print(" newDistanceAfterAdjustment");
    Serial.println(newDistanceForwardAfterAdjustment);
  #endif
  */
}



void loop() {
  // put your main code here, to run repeatedly:
  if (counter==0) {
      counter++;
      
      UltrasonicClass ultrasonicObj;
      LocalizationClass localizationObj;
      DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);    
      MovementsClass movementObj(ultrasonicObj, localizationObj, determineWorldObj);

      float mine = 1.24;
      memTestFunction(mine,mine);

      if (true == false) {
        adjustDistanceToWall(movementObj, ultrasonicObj, 5);
      }

      if (true == true) {
        delay(2000);
        Serial.println("foo");
        Serial.println(ultrasonicObj.distanceAtAngle(0));
        sparki.beep();
        delay(2000);
        Serial.println(ultrasonicObj.distanceAtAngle(90));
        ultrasonicObj.positionServo(0);
        
      }
  }
}
