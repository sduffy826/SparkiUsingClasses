#ifndef _INCL_WORLD
#define _INCL_WORLD
#include <Sparki.h>
#include "sparkiClassCommon.h"

#include "localizationClass.h"
#include "ultrasonicClass.h"

struct WorldCoord {
  float xMax;
  float xMin;
  float yMax;
  float yMin;
};

// Have forward declarations for the UltrasonicClass it pulls this in 
class UltrasonicClass; 

class DetermineWorldClass {
  private: UltrasonicClass *ultrasonicObject;
           LocalizationClass *localizationObject;
           WorldCoord worldCoord;

  public: DetermineWorldClass();
          DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj);
          void calculateRectangularCoordinates();
          void checkWorldCoordinates();   // Call this periodically to check and adjust world coordinates based on current position
          WorldCoord getWorldCoordinates();
          void recordObstacleFromPoseToLength(const Pose &servoPivotPose, const float &distanceToObstacle);
          void showWorld();
          
};
#endif
