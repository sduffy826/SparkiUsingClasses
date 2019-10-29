#ifndef _INCL_ULTRA
#define _INCL_ULTRA

#include <Sparki.h>
#include "sparkiClassCommon.h"

class UltrasonicClass {
  private: int ultrasonicTempInt; // declare once as global to save space
           int ultrasonicAngle = 190; // Init to invalid angle

  public: UltrasonicClass();
          void positionServo(int theAngle);
          int distanceAtAngle(const int &angleOfServo);
          int distanceRight();
          int distanceLeft();
          float getActualBodyDistanceFromFront(const int &reportedDistance);
          float getActualGripperDistanceFromFront(const int &reportedDistance);
          float getActualCenterOfBodyDistanceFromFront(const int &reportedDistance);
          float getActualBodyDistanceFromSide(const int &reportedSideDistance);
          float getActualCenterOfBodyDistanceFromSide(const int &reportedSideDistance);
          float getAdjustedUltrasonicReadingAfterRotation(const int &reportedDistance, const bool &nowAtZeroAngle);
          void showUltrasonic(const int &theAngle, const int &theDistance);
};
#endif
