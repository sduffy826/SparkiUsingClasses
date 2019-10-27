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
          unsigned int getMillisToGetThere(const float &distanceInCM);
          void startMoving(const bool &goForward);
          float getDistanceTraveledForTime(const int &milliseconds);
          float getDistanceTraveledSoFar();
          void stopMoving();
          boolean moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance);
          boolean moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance);
          void turnLeft(const int &degrees);
          void turnRight(const int &degrees);
          int getClosest90Angle();
          void turnToAngle(const int &theAngle);
          int getDistanceAtAngle(const int &angle);
          void turnToZero();
          void turnTo90ClosestDegreeOrientation();
          int adjustDistanceToWall(const int &desiredDistance, const int &currentWallDistance);
          float wallOpeningDistance(int &distanceToMoveForward, const int &startWallDistance, const int &lastWallDistance);
          
          void followWall();
          void showTurnRadius();
          void showWallMovements();
};
#endif
