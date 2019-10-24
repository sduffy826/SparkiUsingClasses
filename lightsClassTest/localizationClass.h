#ifndef _INCL_LOCAL
#define _INCL_LOCAL

#include <Sparki.h>
#include "sparkiClassCommon.h"

struct Pose {
  float xPos;
  float yPos;
  float angle;
};

class LocalizationClass {
  private: Pose pose;

  public: void setPose(float x, float y, float angle);
          Pose getPose();
          void setCurrentAngle(float angle);
          float getCurrentAngle();
          void setCurrentXPosition(float x);
          float getCurrentXPosition();
          int getCurrentXPositionInMM();
          int getCurrentXPositionInCM();
          void setCurrentYPosition(float y);
          float getCurrentYPosition();
          int getCurrentYPositionInMM();
          int getCurrentYPositionInCM();
          float degreesToRadians(float degrees);
          byte getQuadrantAngleIsIn(float degrees);
          float getAngle(float angleInDegrees);
          float calculateRealAngleWithAdjustment(float angleDelta);
          float getMidpointBetweenTwoAngles(float angle1, float angle2);
          float calculateNewX(float distance, float angleDelta);
          float calculateNewY(float distance, float angleDelta);
          void setNewPosition(float distance, float angleDelta);
          float getShortestAngleDeltaToGetToOrientation(float targetOrientation);
          void showLocation();
};
#endif
