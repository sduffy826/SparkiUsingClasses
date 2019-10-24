#ifndef _INCL_WORLD
#define _INCL_WORLD
#include <Sparki.h>
#include "sparkiClassCommon.h"

#include "localizationClass.h"
#include "ultrasonicClass.h"

class DetermineWorldClass {
  private: UltrasonicClass ultrasonicObject;
           LocalizationClass localizationObject;
  
           float worldXDimension;
           float worldYDimension;

  public: DetermineWorldClass();
          DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj);
          void adjustWorldCoordinate(float newX, float newY);
          float getWorldXDimension();
          float getWorldYDimension();
          void showWorld();
          void calculateRectangularCoordinates();
};
#endif
