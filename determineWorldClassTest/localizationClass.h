#ifndef _INCL_LOCAL
#define _INCL_LOCAL

#include <Sparki.h>
#include "sparkiClassCommon.h"

struct Pose {
  float xPos;
  float yPos;
  int angle;
};

class LocalizationClass {
  private: Pose pose;

  public: convertCoordinateSystemAngle(const int &angle);
          void setPose(const float &x, const float &y, int const &angle);
          Pose getPose();
          void setCurrentAngle(const int &angle);
          int getCurrentAngle();
          void setCurrentXPosition(const float &x);
          float getCurrentXPosition();
          int getCurrentXPositionInMM();
          int getCurrentXPositionInCM();
          void setCurrentYPosition(const float &y);
          float getCurrentYPosition();
          int getCurrentYPositionInMM();
          int getCurrentYPositionInCM();
          float degreesToRadians(const int &degrees);
          int radiansToDegrees(const float &radians);
          byte getQuadrantAngleIsIn(const int &degrees);
          int getAngle(int angleInDegrees);
          int calculateRealAngleWithAdjustment(const int &angleDelta);
          int getMidpointBetweenTwoAngles(const int &angle1, const int &angle2);
          float calculateNewX(const float &distance, const int &angleDelta);
          float calculateNewY(const float &distance, const int &angleDelta);
          void setNewPosition(const float &distance, const int &angleDelta);
          int getShortestAngleDeltaToGetToOrientation(const int &targetOrientation);
          void showPose(const Pose &pos2Show);
          void showLocation();
          void writeMsg2Serial(char *theMessage);  // Little helper method
          float distanceBetweenPoses(const Pose &firstPose, const Pose &secondPose);
          Pose calculatePose(const Pose &thePos, const int &angleOfMovement, const int &distanceMoved);
          Pose triangulatePoses(const Pose &firstPose, const Pose &secondPose, const int &angleOfMovement);
};
#endif
