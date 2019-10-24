#ifndef _INCL_LIGHT
#define _INCL_LIGHT

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "movementsClass.h"
#include "ultrasonicClass.h"
#include "localizationClass.h"
#include "determineWorldClass.h"

#define DEBUGLIGHTS false

#define LIGHTSAMPLEANGLE 30
#define LIGHTSAMPLESIZE 3
#define LIGHTSAMPLEDELAY 50
#define LIGHTSAMPLEVALUE2USE 1  // The median value
#define LIGHTCALIBRATIONARRAYSIZE (360/30)  // Degrees / Sample Angle
#define LIGHTANGLERANGETOIGNORE 90 // Degrees to ignore of an existing light source, this is ignore left and right (so it's double)
#define LIGHTSINWORKSPACE 2 // Can't be more than 2 for now
#define LIGHTMINTRIANGULATION 6 // Min cm to be able to triangulate angle
#define LIGHTOPTIMALTRIANGULATION 10  // CM for best triangulation

struct LightAttributes {
  unsigned int lightLeft : 10;
  unsigned int lightCenter : 10;
  unsigned int lightRight : 10;
  unsigned int flag1 : 1;
  unsigned int flag2 : 1;
};

// Struct to hold the pct change 0-100
struct LightDeltaPct {
  unsigned int leftPct : 7;
  unsigned int centerPct : 7;
  unsigned int rightPct : 7;
  unsigned int leftPos : 1;
  unsigned int centerPos : 1;
  unsigned int rightPos : 1;
};

class LightsClass {
  private: MovementsClass movementsObj;
           UltrasonicClass ultrasonicObj;
           LocalizationClass localizationObj;
           
           LightAttributes lightCalibration[LIGHTCALIBRATIONARRAYSIZE];  // Must coincide with sample angle
           LightDeltaPct lightDeltaPcts[LIGHTCALIBRATIONARRAYSIZE];     // Readings
           LightAttributes lightSample[4]; 

  public: LightsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, MovementsClass &movementsObject);
          void setLightAttributes();
          void showLightAttributes(int theAngle);
          void sampleWorldLights();
          int getDeltaPct(int currentValue, int calibrationValue);
          void calculateLightDeltas();
          int deltaPctHelper(int deltaPct, bool isPositive);
          bool numberBetweenRange(int theNum, int lowValue, int highValue);
          int getAngleWithBiggestLightDelta(int multFactor, int angle2IgnoreStart, int angle2IgnoreEnd);
          int getAngleWithHighestLightDelta(int angle2IgnoreStart, int angle2IgnoreEnd);
          int getAngleWithLowestLightDelta(int angle2IgnoreStart, int angle2IgnoreEnd);
          void showLightDirection(int theAngle);
          void setPotentialLightTargets();
};
#endif
