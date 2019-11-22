#ifndef _INCL_MOVE
#define _INCL_MOVE

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "localizationClass.h"
#include "ultrasonicClass.h"
#include "determineWorldClass.h"

struct MovementAttributes {
  unsigned int startTime;
  unsigned int elapsedStateChange;  // Log what the elapsed time was when the state changed
  unsigned int timeSinceLocalized;  // Has the time last localized... use this to localize while moving
  unsigned int amMoving : 1;
  unsigned int wallOpening : 1;
  unsigned int rightWall : 1;
  unsigned int inReverse : 1;
  unsigned int reserved : 4; // bits avail within boundry
};

class MovementsClass {
  private: MovementAttributes movementState;
           UltrasonicClass *ultrasonicObj;
           LocalizationClass *localizationObj;
           DetermineWorldClass *determineWorldObj;
           //void handleWallChange(float startWallDistance, float currWallDistance, bool rightWall);
           void setLocalizationPosition(const boolean &inReverse);

  public: MovementsClass::MovementsClass();
          MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject);

          // Called when you need to adjust your distance to a wall
          float adjustDistanceToWall(const float &desiredDistance, const float &currentWallDistance);

          // Logic to follow the wall, it'll go thru openings if it can
          void followWall();

          int getClosest90Angle();  // Get the closest 90' from your current pose

            // Return the distance at this angle; note this moves the robot that's why it's here and not in the ultrasonic class
          float getDistanceAtAngle(const int &angle);
          
          // Get the distance you'd travel if moving for this number of milliseconds
          float getDistanceTraveledForTime(const int &milliseconds);

          // Get the distance traveled since you started started moving... this returns 0 if you aren't currently moving
          float getDistanceTraveledSoFar();
         
          // Get the elapsed time since you started moving
          unsigned int getElapsedSinceStartedMoving();

          // Return the number of milliseconds it'll take to cover a given distance
          unsigned int getMillisToGetThere(const float &distanceInCM);
          
          // Initialization (this is called from constructor, but can be used to 'reset' state... be careful)
          void initMovements();
          
          // Return boolean with 'moving' status
          bool isMoving();
          
          // Routines to handle movement forward and backward
          boolean moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance);
          boolean moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance);
          
          // Move to a pose... currently this moves in a straight line there... make it smarter
          void moveToPose(const Pose &targetPose);

          // Output/debug methods
          void showTurnRadius();
          void showWallMovements();

          void startMoving(const bool &goForward);
          void stopMoving();

          void turnLeft(const int &degrees);
          void turnRight(const int &degrees);
          
          void turnToAngle(const int &theAngle);
          void turnToZero();
          void turnToClosest90DegreeOrientation();
         
          float wallOpeningDistance(float &distanceToMoveForward, const float &startWallDistance, const float &lastWallDistance);
          
          
          
          
};
#endif
