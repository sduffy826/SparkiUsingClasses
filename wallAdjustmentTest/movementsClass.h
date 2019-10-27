#ifndef _INCL_MOVE
#define _INCL_MOVE

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "localizationClass.h"
#include "ultrasonicClass.h"
#include "determineWorldClass.h"

struct MovementAttributes {
  unsigned int startTime;
  unsigned int elapsedStartOpening;  // The elapsed time that we saw start of opening
  unsigned int amMoving : 1;
  unsigned int wallOpening : 1;
  unsigned int rightWall : 1;
  unsigned int inReverse : 1;
  unsigned int reserved : 4; // bits avail within boundry
};

class MovementsClass {
  private: MovementAttributes movementState;
           UltrasonicClass ultrasonicObj;
           LocalizationClass localizationObj;
           DetermineWorldClass determineWorldObj;
           void handleWallChange(int startWallDistance, int currWallDistance, bool rightWall);

  public: MovementsClass::MovementsClass();
          MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject);
          void initMovements();
          unsigned int getElapsed();
          unsigned int getMillisToGetThere(float distanceInCM);
          void startMoving(bool goForward);
          float getDistanceTraveledSoFar();
          float getDistanceTraveledForTime(int milliseconds);
          void stopMoving();
          boolean moveBackward(float distanceToTravel, float minAllowedDistanceToObstacle, bool checkFrontDistance);
          boolean moveForward(float distanceToTravel, float minAllowedDistanceToObstacle, bool checkFrontDistance);
          void turnLeft(byte degrees);
          void turnRight(byte degrees);
          int getClosest90Angle();
          void turnToAngle(int theAngle);
          int getDistanceAtAngle(int angle);
          void turnToZero();
          void turnTo90ClosestDegreeOrientation();
          float wallOpeningDistance(int &distanceToMoveForward, int &startWallDistance, int &lastWallDistance);
          void adjustDistanceToWall(int &newDistanceForwardAfterAdjustment, int &desiredDistance, int &currentWallDistance);
          void followWall();
          void showTurnRadius();
          void showWallMovements();
};
#endif
