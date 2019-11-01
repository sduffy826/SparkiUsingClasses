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

class DetermineWorldClass {
  private: UltrasonicClass *ultrasonicObject;
           LocalizationClass *localizationObject;
           WorldCoord worldCoord;

  public: DetermineWorldClass();
          DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj);
          WorldCoord getWorldCoordinates();
          void checkWorldCoordinates();   // Call this periodically to check and adjust world coordinates based on current position
          void showWorld();
          void calculateRectangularCoordinates();
};
#endif
