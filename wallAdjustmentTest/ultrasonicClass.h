#ifndef _INCL_ULTRA
#define _INCL_ULTRA

#include <Sparki.h>
#include "sparkiClassCommon.h"

class UltrasonicClass {
  private: int ultrasonicTempInt; // declare once as global to save space
           int ultrasonicAngle = 190; // Init to invalid angle

  public: UltrasonicClass();
          void positionServo(int theAngle);
          int distanceAtAngle(int angleOfServo);
          int distanceRight();
          int distanceLeft();
          void showUltrasonic(int theAngle, int theDistance);
};
#endif
