#ifndef _INCL_MOVE
#define _INCL_MOVE

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "localizationClass.h"
#include "ultrasonicClass.h"
#include "determineWorldClass.h"

struct MovementAttributes {
  unsigned int startTime;
  unsigned int amMoving : 1;
  unsigned int reserved : 7; // bits avail within boundry
};

class MovementsClass {
  private: MovementAttributes movementState;
           UltrasonicClass ultrasonicObj;
           LocalizationClass localizationObj;
           DetermineWorldClass determineWorldObj;

  public: MovementsClass::MovementsClass();
          MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject);
          void initMovements();
          unsigned int getElapsed();
          unsigned int getMillisToGetThere(float distanceInCM);
          void startMovingForward();
          float getDistanceTraveledSoFar();
          void stopMoving();
          boolean moveForward(float distanceToTravel, float minAllowedDistanceToObstacle);
          void turnLeft(byte degrees);
          void turnRight(byte degrees);
          int getClosest90Angle();
          void turnToAngle(int theAngle);
          int getDistanceAtAngle(int angle);
          void turnToZero();
          void turnTo90ClosestDegreeOrientation();
          void followWall();
          void showTurnRadius();
          void showWallMovements();
};
#endif
