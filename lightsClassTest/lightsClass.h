#ifndef _INCL_LIGHT
#define _INCL_LIGHT

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "movementsClass.h"
#include "ultrasonicClass.h"
#include "localizationClass.h"
#include "determineWorldClass.h"

#define DEBUGLIGHTS false
#define DEBUGLIGHTDELTA false

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
  unsigned int leftSignBit : 1;
  unsigned int centerSignBit : 1;
  unsigned int rightSignBit : 1;
};

class LightsClass {
  private: MovementsClass *movementsObj;
           UltrasonicClass *ultrasonicObj;
           LocalizationClass *localizationObj;
           
           LightAttributes lightCalibration[LIGHTCALIBRATIONARRAYSIZE];  // Must coincide with sample angle
           LightDeltaPct lightDeltaPcts[LIGHTCALIBRATIONARRAYSIZE];     // Readings
           LightAttributes lightSample[4]; 

           void setLightAttributesAtCurrentPose();  // Takes light samples at current pose and updates lightSample array; the array is sored so you can use media value
           int getAngleWithBiggestLightDelta(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd);  // Get largest/smallest delta 
           int getAngleWithHighestCurrentReading(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement); // Get bright or dim angle

  public: LightsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, MovementsClass &movementsObject);
          LightAttributes getLightAttributesAtCurrentPose();  // Returns light attributes for the current pose you are at (this takes a sample)
          void showLightAttributes(char *msgStr, const LightAttributes &liteAttr, const int &theAngle);  // Good for debugging, shows light attributes for argument
          void showSampledLightAttributes(const int &theAngle);    // Shows the value that was last 'sampled' 
          void showCalibrationLightAtAngle(const int &theAngle);  // Show the calibration light attributes for the angle we want
          void sampleWorldLights();  // Takes a sample of the 'world lights' and stores values in the lightCalibration array (for respective angles)
          int getLightDeltaPctBetween2Values(const int &currentValue, const int &calibrationValue);
          void calculateLightDeltas();  // This updates the lightDeltaPcts array with the difference between the 'world' lights now and the original values taken
          int deltaPctHelper(const int &deltaPct, const bool &isPositive);  // Helper method to return the delta pct as an integer
          bool numberBetweenRange(const int &theNum, const int &lowValue, const int &highValue);  // Helper to ensure a number is between a given range
          
          int getAngleWithHighestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd);  // Gets the angle that is, note the angle returns is in the world coordinate
          int getAngleWithLowestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd); // Similar to above but gets lowest or darkest change
          void showLightDeltaPctForAngle(const int &theAngle); // Little helper
          
          int getAngleWithDimmestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement);  // Gets dimmest 'current' light
          int getAngleWithBrightestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement);  // Gets brightest light
          void setPotentialLightTargets();  // This tries to triangulate your target position to lights... cool work but lights are not good for triangulation :(
};
#endif
