#ifndef _INCL_INFRA
#define _INCL_INFRA

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "movementsClass.h"
#include "ultrasonicClass.h"
#include "localizationClass.h"
#include "determineWorldClass.h"

#define DEBUGINFRARED false

#define INFRARED_LINE_THRESHOLD .10  // .10 Means 10%, so if 10% diff from base it'll register as a line
#define INFRARED_MIN_READING 300
#define INFRARED_LINE_WIDTH 2.3     // Width of tape... have it use method
#define INFRARED_SAMPLE_SIZE 10      // Number of samples to take
#define INFRARED_SENSOR_FORWARD_OF_CENTER 4
#define INFRARED_DRIFT_ADJUSTMENT_DEGREES 3  // Amount of degrees to adjust when drifting off line

struct InfraredAttributes {
  unsigned int edgeLeft : 10;
  unsigned int lineLeft : 10;
  unsigned int lineCenter : 10;
  unsigned int lineRight : 10;
  unsigned int edgeRight : 10;
  unsigned int el_line : 1;     // Flags that say if detect line
  unsigned int ll_line : 1;     // It uses the base value to 
  unsigned int lc_line : 1;     // determine this
  unsigned int lr_line : 1;
  unsigned int er_line : 1;

  unsigned int driftLeft : 1;   // Flags for state
  unsigned int driftRight : 1;
  unsigned int onLine : 1;
  unsigned int atExit : 1;
  unsigned int startLeftPath : 1;
  unsigned int startRightPath : 1;
  unsigned int endLeftPath : 1;
  unsigned int endRightPath : 1;
  unsigned int atEntrance : 1;
};

class InfraredClass {
  private: MovementsClass *movementsObj;
           LocalizationClass *localizationObj;
           
           InfraredAttributes infraredBase;    // Base reading without anything underneath it
           byte lineWidthInMM;

           // Helper method to identify if infrared lights are on a line
           boolean lineFlagHelper(const int &currentReading, const int &baseReading);
           
   public: InfraredClass(LocalizationClass &localizationObject, MovementsClass &movementsObject);

           // Adjust for drifting
           void adjustForDrifting(const bool &driftingLeft);

           // Struct assignments
           void assignSourceAttributesToTarget(const InfraredAttributes &source, InfraredAttributes &target);
            
           
           // Clear infrared readings for the argument passed in
           void clearInfraredAttributes(InfraredAttributes &attr2Clear);

           // Get the base attributes
           InfraredAttributes getBaseAttributes();

           // Get infrared attributes at the current pose
           InfraredAttributes getInfraredAttributesAtCurrentPose();  

           // Return tape width in cm (as float)
           float getLineWidth();

           // Return the pose of the middle infrared sensor
           Pose getPoseOfCenterSensor();
           
           // Routines to sample the infrared readings
           void setInfraredBaseReadings();  

           // Debugging method - show the attributes passed in 
           void showInfraredAttributes(char *msgStr, const InfraredAttributes &attr);

           // Return boolean if the state changed 
           bool stateChanged(InfraredAttributes &currAttr, const InfraredAttributes &priorAttr);

           void waitForInstructions();
          
};
#endif
