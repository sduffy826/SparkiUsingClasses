#ifndef _INCL_INFRA
#define _INCL_INFRA

#include <Sparki.h>
#include "sparkiClassCommon.h"
#include "movementsClass.h"
#include "ultrasonicClass.h"
#include "localizationClass.h"
#include "determineWorldClass.h"
#include "QueueArray.h"

#define DEBUGINFRARED false

#define WRITEMAPDATA true

#define INFRARED_LINE_THRESHOLD .05  // .10 Means 10%, so if 10% diff from base it'll register as a line
#define INFRARED_MIN_READING 1       // Had to make it very small, on the gaffer tape the values are < 100
#define INFRARED_LINE_WIDTH 4.7     // Width of tape... have it use method
#define INFRARED_SAMPLE_SIZE 7      // Number of samples to take
#define INFRARED_TAPE_SAMPLE_SIZE 3  // Number of samples to take when adjusting width base on tape
#define INFRARED_DELAY_BETWEEN_SAMPLES 6  // Milliseconds to delay between samples
#define INFRARED_SENSOR_FORWARD_OF_CENTER 4
#define INFRARED_DRIFT_ADJUSTMENT_DEGREES 3  // Amount of degrees to adjust when drifting off line
#define INFRARED_MAX_GOAL_DISTANCE 10        // Max distance to be considered a goal (i.e. if obstacle detected < 10cm it's a goal position
#define INFRARED_MAX_DISTANCE_TO_TRAVEL 100.0  
#define INFRARED_CONSECUTIVE_EDGE_READINGS 3 
#define INFRARED_INTERVAL_2_CHECK_ON_LINE 1    // Interval to check that we're on the line after a position change
#define INFRARED_SPACE_BEFORE_LINE_CHECK 12  // Need to move this amount before we'll try to adjust for line (1/2 width + tape width) about


#define EXPLORE_MODE 'X'  // eXplore
#define GOAL_MODE 'G'     // Goal
#define DONE_MODE 'Q'     // done/Quit
#define GOTO_MODE 'M'     // goto/Move
#define SETPOSE_MODE 'P'  // set robot pose


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

struct InfraredInstructions {
  // InfraredInstructions();
  char instruction;  // X-explore, G-goal, Q-quit/done M-moveto (goto)
  Pose pose;         // has x, y (floats) and angle (int)
};
//InfraredInstructions::InfraredInstructions() { }


class InfraredClass {
  private: MovementsClass *movementsObj;
           LocalizationClass *localizationObj;
           
           InfraredAttributes infraredBase;    // Base reading without anything underneath it
           byte lineWidthInMM;
           unsigned int millisFor90Degrees;    // Milliseconds needed to turn 90'

           // Helper method to identify if infrared lights are on a line
           bool lineFlagHelper(const int &currentReading, const int &baseReading);
           
   public: InfraredClass(LocalizationClass &localizationObject, MovementsClass &movementsObject);

           // Move to an angle and return the distance to get from current pose to target pose 
           // NOTE!!!! may want to move this into movementsClass (and change the move2Pose to
           // use this... 
           float adjustAngleToPose(const Pose &targetPose);

           // Routine below adjusts the position of the robot on the line, pass in the last interval that was
           // used... it'll return the new interval that should be checked, NOTE: robot should not be moving
           // when this is called.
           int adjustPositionOnLine(const int &lastIntervalDistance);

           // Adjust for drifting
           void adjustForDrifting(const bool &driftingLeft);

           // Another method to adjust to center of line :)
           void adjustToLineCenter();

           // Helper routine for 'adjustToTape'
           void adjustToTapeHelper(bool &leftEdge, bool &rightEdge);
           
           // Routine to adjust to center of tape, this one rotates 90' and positions itself
           // so it's perpenticular to the tape, then it rotates back onto the center of the tape
           bool adjustToTape();

           // Struct assignments, don't need structs are value types so assiging them is a member wise value assignment
           // void assignSourceAttributesToTarget(const InfraredAttributes &source, InfraredAttributes &target);
            
           // Calculate the angle from the edge reading to the tape, if firstArg is true then you want to calculate
           // the left edge otherwise it'll do the right one, the other args are the 'base' reading and the number
           // of milliseconds needed to turn 90 degrees (I use that to calculate the angle)
           int calcAngleFromEdgeToTape(const bool &leftEdge, const int &baseReading, const int &millisFor90Degrees, const bool &isRecall=false);

           // Clear infrared readings for the argument passed in
           void clearInfraredAttributes(InfraredAttributes &attr2Clear);

           // Extract value in src delimitted by , result goes into dst
           const char* extractToken(char* dst, const char* src);

           // Get the base attributes
           InfraredAttributes getBaseAttributes();

           // Get infrared attributes at the current pose
           //InfraredAttributes getInfraredAttributesAtSensorPose();  
           void getInfraredAttributesAtSensorPose(InfraredAttributes &infraAttr);

           // Return tape width in cm (as float)
           float getLineWidth();

           // Return the pose of the middle infrared sensor
           Pose getPoseOfCenterSensor();

           // Return bool if the robot is on an intersection (both edge readings on)
           bool onIntersection(const int &baseReading);

           // Return bool if any of the three center infrared lights is on
           bool onLine(const int &baseReading);

           // Routine to get parms returned on serial device... may want to move this into a serial class down the road
           void parmCountAndLength(const char* str_data, unsigned int& num_params, unsigned int& theLen);

           // Util to read a char from the serial port (should move into a serialClass obj down the road)
           int readFromSerialPort();
 
           // Routines to sample the infrared readings
           void setInfraredBaseReadings();  

           // Debugging method - show the attributes passed in 
           void showInfraredAttributes(char *msgStr, const InfraredAttributes &attr, const Pose &poseOfCenterSensor, const bool &isGoalPosition);

           // Return bool if the state changed 
           bool stateChanged(InfraredAttributes &currAttr, const InfraredAttributes &priorAttr);

           // Wait for instructions from the computer... it returns bool True if it got them :)
           bool waitForInstructions(QueueArray<InfraredInstructions> &queueOfInstructions);    
          
};
#endif
