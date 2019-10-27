#ifndef _INCL_LIGHT
#define _INCL_LIGHT

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "movementsClass.h"
#include "ultrasonicClass.h"
#include "localizationClass.h"
#include "determineWorldClass.h"

#define DEBUGLIGHTS false

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
          void showLightAttributes(const int &theAngle);
          void sampleWorldLights();
          int getDeltaPct(const int &currentValue, const int &calibrationValue);
          void calculateLightDeltas();
          int deltaPctHelper(const int &deltaPct, const bool &isPositive);
          bool numberBetweenRange(const int &theNum, const int &lowValue, const int &highValue);
          int getAngleWithBiggestLightDelta(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd);
          int getAngleWithHighestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd);
          int getAngleWithLowestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd);
          void showLightDirection(const int &theAngle);
          void setPotentialLightTargets();
};
#endif
