#ifndef _INCL_ULTRA
#define _INCL_ULTRA

#include <Sparki.h>
#include "sparkiClassCommon.h"

class UltrasonicClass {
  private: int ultrasonicAngle = 190; // Init to invalid angle

  public: UltrasonicClass();
          void positionServo(int theAngle);
          
          // Get the servo angle
          int getServoAngle();

          // Get distance reading, this is from the servo's position
          float getDistanceFromServoAtAngle(const int &angleOfServo); 

          // Return the distance from the center of the robot to the obstacle
          float getDistanceFromCenterOfRobotToObstacle(const int &angleOfServo);

          // Returns the free space in front of the robot, this does not include the space the gripper's taking... this is useful when trying to grab 
          // an object between the grippers
          float getFreeSpaceInFrontExcludingGripper(const int &angleOfServo);

          // Get the space thats fee in front of the grippers
          float getFreeSpaceInFrontOfGripper(const int &angleOfServo);

          // Get the free space between the robot and the wall on the right
          float getFreeSpaceOnRight();

          // Get free space between robot and wall on the left
          float getFreeSpaceOnLeft();

          // Get the tolerance of the sensor (this is the +/- from the reading it reports)
          float UltrasonicClass::getSensorTolerance();
          
          void showUltrasonic(const int &theAngle, const float &theDistance);
};
#endif
