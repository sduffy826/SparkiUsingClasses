## General
- The work here was performed from various assignments with the arcbotics 'sparki' robot (and more assignments are coming).  This is a fluid repository and I'm making changes very frequently (at least for the next 6 weeks).  When I get closer to the finish line I'll make sure that the documentation/code is 'cleaner' and more accurate, but you should be able to get a feel for where I am and what I'm doing.  Some of the info toward the beginning of this 'readme' may not be as interesting as the classes/methods; if you want, read the fist bullet under 'NOTES" and then jump down to the Code/Class Definition section.

- Right now the documentation is geared toward showing the classes/methods developed.  Most of the logic to satisfy the assignments are in the .ino (main) programs; they're not specifically documented here; that'll be coming 


## NOTES

- Here's a brief overview of includes you should be interested in.  I encapsulated the classes to expose public methods for dealing with a given area... i.e. any code that wants to find out the distance an obstacle is using the ultrasosnic sensor will do that by calling methods within the ulltrasonicClass.  The associated .cpp's have implementation of those methods.
  - **sparkiClassCommon.h** has common definitions and global variable definitions; you should be able to control a lot of functionality by just tweaking the 'defines' listed here
  - **determineWorldClass.h** code for managing the robots world, eventually will have map with all the attributes in it
  - **lightsClass.h** code related to dealing with the light sensors, calibration, sampling, calculating changes etc..
  - **localizationClass.h** code related to localization, has the robots pose and there are a lot of methods here to calculate positioning (more below).
  - **movementsClass.h** code to manage robot movements (forward, backward, turning...), has logic for following walls, adjusting positioning, etc...
  - **ultrasonicClass.h** code fo interfacing with the ultrasonic sensor
- The '*.ino' code in each of the directories has code for testing functionality... since there can only be one .ino file I have conditions like #if XXXX #endif, or if (true == false) .. to control whether that block of code is executed when the .ino program is launched.  Eventually I'll redesign that, but this is a work in progress so that's lower priority right now :)

- I tried to make common code (include files) but couldn't get arduino ide to recognize the path to different directories.  It appears the code has to be in the directory that wants to use it.  What I did is have the code for an area in a .h & .cpp file in the directory for that function.  I named the directory with a 'Test' suffix (i.e. directory localizationClassTest is the directory for the test program (.ino) and where it's localizationClass.h and localizationClass.cpp files are.  To use that code in a different area (i.e. determineWorldTest) you copy the .h & .cpp files into the determineWorldTest directory.  I created a **'syncClassFiles.sh'** script in git/root path, when you run it it copies the required files into the directories that need them.  When you write a new area just update it to include the .h/.cpp files you want to use.  MAKE SURE you keep it up to date, and only update the .h/.cpp file in the correct location.  Also, you need to close/open the folder after you've run the shell script (the ide doesn't know if underlying file was updated).

- Memory is a concern, you'll probably want to uncomment NO_LCD, NO_ACCEL and NO_MAG in the Sparki.h file.  You should also set USE_LCD to false in sparkiClassCommon.h... if that's false then (most of) the code will output values to the serial port instead of the lcd screen; if it's true it'll try to write the debugging/log info to the lcd

## To Do's
- Check the methods added to ultrasonic, determine world to track obstacles... done for mapping
- Update the determineWorld class to write obstacle information to serial so computer can keep track of map
- Check legend abbreviations
- Review this along with code and put in extra documentation where needed
- Create a 'wander world' or explore world routine
- Couldn't get light triangulation to work, try it with flashlights that can give a more pinpoint/direct light source
- A light logic test, to find light may want to use shadow logic... i.e. if light is low then the ultrasonic sensor will cause a shadow on light sensors... maybe moving the sensor at different angles can help identify light source
- Revisit using accelerometer to detect collisions... at first I couldn't get working but try again
- Recheck data types (i.e us struct and define var as int : &lt;bitSpec&gt;;)

## Code for testing/calibration (most of this is in the ../sparki directory, and the sparki repo)
- **movementTest**: This has code used to test movements; I used this to calculate how fast sparki is (2.78cm/sec); to do this you can use the moveForward function, it'll show the elapsed time to move that distance.  I found that sparki was off; it actually moved further than requested.  See the sparkiMovementAnalysis spreadsheet in the sparki_python repo for calculations I did.
- **sensorTest**: Code for testing sensors; it outputs a bunch of values from the sensors... have the python code reading the serial port so that it can provide mean, median, mode, std deviation for the various sensors.
- **PlaygroundToTest**: Just stub for testing various functions, a play area :)
- **wallAdjustmentTest** Code I was using to try and adjust when the sparki is moving away from a wall at an angle
  - Note, methods are: 
    - To wiggle toward the wall (gripper should be wide), and continue to do that, what happens is one of the grippers hit the wall so it can't move forward in that direction, and eventually the robot will get in a position where he's 90' to the wall; you then just backup, and turn 90'... you're now parallel to the wall
    - You need to have enough distance to do this, so keep track of how long the wall has been next to you... when you see a state change in distance you log where you are... you continue to move forward till it hits the next increment; you now have a distance traveled in x direction, and delta in the y direction; you can calculate the angle that you are offset by.... you then backup till you're at desired distance to the wall (note may want to go little more to account for +- of the sensor), you then stop and turn using the angle you just calculated.
  
## LEGEND Serial communications
**Memory** is critical and strings take up quite a bit of memory, I used the abbreviations below when outputting to the serial port.  The first letter is related to the class that generated the value (i.e D-Determine World, L-Lights, P-Localization, M-Movements U-Ultrasonic)
- **DW** Define world, world dimensions (i.e. DW,x1,0.00,x2,115.40,y1,0.00,y2,151.40), it gives x1/x2 (min/max) so you know length of field, then y coordinates; note as it expllores the world these values could change and x1, y1 may go negative
- **DP** Position you are in the world (i.e. DP,x,68.20,y,30.20)
- **DO** Shows the pose and distance that an obstacle was found at. (i.e. DO,x,10.3,y,20.5,<,160,d,7.3)

---
- **IR** Infrared sensors.  There are five infrared sensors underneath the sparki.  There's a structure (InfraredAttributes) that stores a bunch of information about the sensors (it's 8 bytes long - Sensor readings are each 10 bytes, all the other values are bit(1)).  The reason we have so much data is that the intent is to have the sparki communicate with the computer and the computer can use this information to map the configuration space and also instruct the sparki on paths to take.  The format of the data written to the serial device is
```
 'IR,StateChg,el,980,ll,980,lc,888,lr,684,er,980,ell,0,lll,0,lcl,0,lrl,1,erl,0,sdl,1,sdr,0,sol,0,sae,0,slp,0,srp,0,sel,0,ser,0,sas,1'
```
There's a description of the elements below (the value after IR is a description 'StateChg' meaning a state changed was detected).  Note: internally the sparki compares the last reading to the current one in order to detect state changes... and worth mentioning the line detection logic compares the current readings to a base reading taken when the program starts.
| Tag | Value |
| -------- | -------------- |
| Sensor Readings |
| el | Edge Left Infrared Reading (0->1000) |
| ll | Line Left reading |
| lc | Line Center |
| lr | Line Right |
| er | Edge Right Infrared Reading |
| Line Detection|
| ell | Line detected on left edge sensor |
| lll | Line detected for Line Left |
| lcl | Line detected - Center |
| lrl | Line detected - Right |
| erl | Line detected at right edge |
| State Info/Changes |
| sdl | Drifting Left |
| sdr | Drifting Right |
| sol | On Line - detect line but not 'drifting'; keep moving :) |
| sae | At Exit - At end of line (and not an intersection (path left or right) |
| slp | Start detecting 'Path on Left' |
| srp | Start detecting 'Path on right' |
| sel | End detecting the 'Path on Left' (difference between slp and elp is width of path (i.e. tape width)) |
| ser | End detecting the 'Path on Right' |
| sas | Detected 'At Start' of maze (Should only register once when moving onto grid) |

 
---

- **LA** Light angle, has description of what it is (Sam-Sample, Cal-Calibration, Orig-Original, Curr-Current), it's angle, brightness for left, center and right light sensor (i.e LA,Sam,<,0,l,801,c,704,r,787)  Note: the Orig is a 'base light attribute that's saved', we do that if we want to compare one positions light to another (i.e a 'Curr' one)
- **LP** Light brightness delta percentage, it is the percentage that the sample is away from the original/calibrated sample.  An example of this is 'LP,<,60,l,8,c,21,r,-1', it means at 60' the left light is 8% over calibrated light, center is 21% and the right light is -1% (i.e. darker).  
- **LPA** When DEBUGLIGHTS is on you'll see these; it's the same as **LP** but it will show you all angles
- **LD** This shows the "LightsDeltaSum" values; it has the sum of the light delta's over a given sample size.  The format of the output after 'LD,' is &lt;light&gt;,&lt;deltaSum&gt;/&lt;deltaCounter&gt; ...  It's probably good to have an example here.  In the example below it's showing the LA values also:
```
    LA,Orig,<,10,l,715,c,654,r,697

    LA,Curr,<,0,l,714,c,654,r,714
    LD,l,-1/-1,c,0/0,r,17/1

    LA,Curr,<,0,l,723,c,666,r,716
    LD,l,7/0,c,12/1,r,36/2

    LA,Curr,<,0,l,720,c,656,r,704
    LD,l,12/1,c,14/2,r,43/3

    LA,Curr,<,0,l,719,c,657,r,718
    LD,l,16/2,c,17/3,r,64/4

    LA,Curr,<,0,l,728,c,654,r,712
    LD,l,29/3,c,17/3,r,79/5

You can see that each delta from LA,Curr is summed up into &lt;deltaSum&gt;  The &lt;deltaCounter&gt; has a count of each number of increases it's seen (since it's been reset).
```
- **LCD** This is only one when DEBUGLIGHTS is on.  It has the angle, and the  light brightness for the center light, an example is 'LCD,<,120,579' so it means angle 120 has a brightness value of 579.
- These two are related to angles we ignore... when looking for light we sometimes want to ignore other angles... the tags below are showing values for those 'ignored' angles... it's mainly for debugging, and they'll only be displayed when the appropriate DEBUG value is set
  - **LBIR** This shows the angles that will be ignored, it's displayed when DEBUGLIGHTDELTA is on.  An example is 'LBIR,<,120,to,300'; this means that it will ignore light readings between 120' and 300' (inclusive).
  - **LBI** This is shown when DEBUGLIGHT is on; it's meant to 'prove' that an angle was really ignored :)  A sample is 'LBI,<,120'; it means that it really ignored that angle.

---

- **PO** Localization, this outputs the pose that's passed in, x position, y position, &lt; current angle  (lots of whitespace as need to LOOK AT THIS, maybe want a msgString* passed in)

---

- **MR** Shows turn radius (i.e. MR,8.24)

---

- **US** Ultrasonic sensor, this shows you the angle of the servo and the reading it took (i.e. US,<,30,d,13.4)


# Code/Class Definitions 
I listed the classes alphabetically except for the first one... that has the constants so you should know about that one first.

## SparkiClass
- **SparkiCommon**: this has the .h file to be included in code; it defines constants for the sparki like: it's speed, dimensions, etc... you should put constants here (used define instead of const) to save memory.  When you update this run ./syncClassFiles.sh 
  - Note: there's a USE_LCD define in here, if that's false then output from various 'show*' methods will write to the serial port, if it's true then the output will be written to the lcd screen.

## DetermineWorldClass
**Controls/has information about the robots world** This needs to be expanded to map all the obstacles/walls etc in the world; possibly have it communicate with external device that would have storage required.... think more on this
- Constructors
  - **DetermineWorldClass()** - 'default' constructor, required by arduino, but you should initialize object with references to required objects (see next constructor)
  - **DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj)** - Constructor where you pass references to required objects needed within this object.

- Methods that set world coordinates
  - **void calculateRectangularCoordinates()** - Calculates your 'world' space, right now it takes the direction at 90' angles and records them; it also calls localization methods that set's your current position within that world.
  
  - **void checkWorldCoordinates()** - Call this periodically to check and adjust world coordinates based on current position; i.e. when you first called getWorldCoordinates some things may not have been visible; if you call this method periodically (i.e. when you stop) you can adjust the 'world' coordinates.

- Getting the coordinates
  - **WorldCoord getWorldCoordinates()** - Gets your world coordinates

- Mapping world
  - **recordObstacleFromPoseToLength(const Pose &servoPivotPose, const float &distanceToObstacle)** - This is called from the ultrasonic object whenever it sees an obstacle, it calls the determine world object and passes in the pose of the servo axis point and the distance to the obstacle.  Note: I pass the servo axis point and distance so the mapping routine can correctly update all positions between that pose and and endpoint

- Debugging
  - **void showWorld()** - This is just a helper to show your world... mainly for debugging.



## LightsClass
**Provides attributes and routines related to the light sensors** Note it's common to call 'sampleWorldLights()' then turn the lights on (or move) and call 'calculateWorldDeltaPcts()' which will calculate the deltas.  You can then use a method like 'getAngleWithHighestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd)' to find the angle with brighest light (read more below).
```
  // struct for light attributes... since we only have values from 0-1000 we use 10 bits... 
  //   the flag1/flag2 are for future and to make the size of this align on the proper boundary.
  // the array that maintains the 'calibration' settings is defined as 'LightAttributes lightCalibration[LIGHTCALIBRATIONARRAYSIZE]'
  struct LightAttributes {
    unsigned int lightLeft : 10;
    unsigned int lightCenter : 10;
    unsigned int lightRight : 10;
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
  };

  // Struct to hold the pct change 0-100 between the 'world sampled values' (lightCalibration array). 
  // NOTE: we are very memory conscious that's why were doing 'bit' level definitions... if a percentage 
  //   is positive the associated *SignBit will be on (1), if it's negative it will be off.  
  // Note2: use the deltaPctHelper... you pass it the *Pct value and the *SignBit... it'll return what the percentage for you.
  // The array that holds the delta pct's is defined as 'LightDeltaPct lightDeltaPcts[LIGHTCALIBRATIONARRAYSIZE]'
  struct LightDeltaPct {
    unsigned int leftPct : 7;
    unsigned int centerPct : 7;
    unsigned int rightPct : 7;
    unsigned int leftSignBit : 1;
    unsigned int centerSignBit : 1;
    unsigned int rightSignBit : 1;
  };
```

- **private methods**
  - **void setLightSampleAttributesAtCurrentPose()** - Takes light samples at current pose and updates (private) lightSample array; the array is sorted so you can use whatever value we want.  The #define of LIGHTSAMPLEVALUE2USE is the one we use... at the time of this writing it's the median value.
  
  - **int getAngleWithBiggestLightDeltaPct(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd)** - Routine that calculates the angle with the largest delta light percentage; if the multFactor is -1 then it'll return the biggest changes in negative direction, if 1 it'll give you +; the two ignore angles are so you can ignore a range of degrees.   The publicly exposed methods (that call this) are getAngleWithLowestLightDelta and getAngleWithHighestLightDelta.
  
  - **int getAngleWithHighestCurrentReading(const int &multFactor, const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement)** - This routine will take samples of the light and return the brighest light (or dimmest); it does not compare to any prior values... it's just the angle with brighest or lowest intensity.  Note like others it has a range of angles that can be ignored.  NOTE: methods getAngleWithDimmestCurrentLight and getAngleWithBrightestCurrentLight are the publicly exposed methods.. they use this one (best way to encapsulate)

  - **void clearLightDelta(LightDelta &lightDelta)** - Clears a specific LightDelta variable; this is called from clearLightDeltaSum

  - **void setLightDelta(LightDelta &amt, const int &delta)** - Sets the LightDelta values based on the delta amount passed in.  

  - **void setLightDeltaSum(LightDeltaSum &amts, const LightAttributes &original, const LightAttributes &current)** - Uses the two light attributes and updates the variable 'amts' with sum of light deltas and frequence of change

  - **int lightDeltaAmountHelper(const LightDelta &lightDelta1, const LightDelta &llightDelta2, const byte &lightId1, const byte &lightId2, const int &multFactor)** - This is a helper method to compare two LightDeltas and return the id associated with the record that's most significant (you give it the id), the last argument is a multiplication factor... a -1 value means you want the light that decreased more

- **public methods**
  - Routines to sample (calibrate) lights and then determine current lights and updates the pct differences for each angle
    - **void sampleWorldLights()** - Takes a sample of the 'world lights' and stores values in the lightCalibration array (for respective angles)
    
    - **void calculateWorldLightDeltas()** - This updates the lightDeltaPcts array with the difference between the 'world' lights now and the original values taken, note, you should have called 'sampleWorldLights()' before using this one... 
    
  - Routine to get current light attributes (at your current pose).  You can use this to take a sample; move to a new point, take another sample and then compare the two readings to see the net change.
    - **LightAttributes getLightAttributesAtCurrentPose()** - Returns the light attributes at the current pose, this takes a sampling (calls setLightSampleAttributesAtCurrentPose) and returns those values to the caller.

  - Routines related to use 'Light Delta's (not pct's)'
    - **void clearLightDeltaSum(LightDeltaSum &lightDeltaSum)** - Clear the values in a LightDeltaSum variable (i.e. you want to reset it back to original state)

    - **int getLightDeltaSum(LightDelta &lightDelta)** - Helper, gets the total delta associated with the a specific light, this is how you should get the individual values in a LightDeltaSum variable (this handles any conversion needed, so use this :))

    - **int getLightDeltaCounter(LightDelta &lightDelta)** - Another helper, use this to get the frequency of change for a particular light (within a LightDeltaSum), again use this!!  The raw value stored is not the value you want... this handles the conversion to a useful value.
  
    - **getLightAngleToTurnToBasedOnDeltaSum(LightDeltaSum &amts, const LightAttributes &original, const LightAttributes &current)** -  This routine calls method 'setLightDeltaSum' to set the LightDeltaSum variable; it keeps track of the delta amounts between two LightAttributes which are the other args passed in. It then calls method 'lightDeltaAmountsHelper' to determine which light source is the most significant... it does this by seeing which one has changed more frequently (not the light change value, it's frequency).  If two lights have the same frequency of change then it'll return the one with the most significan change.  The value is the angle offset to turn to; a negative value means turn left, a positive means turn right; the angle is defined is LIGHTDELTAANGLE2TURN
  
  - Routines to get the biggest pct change between the calibration amount and the lights after calling 'calculateWorldLightDeltas'
    - **int getAngleWithHighestLightDeltaPct(const int &angle2IgnoreStart, const int &angle2IgnoreEnd)** - This returns the angle (in the world) where the largest light delta exists between the world (lightCalibration) and the values taken after calling 'calculateLightDeltas()'.  You can pass an angle range to ignore... this is useful when you know an area from your current position should be ignored (i.e. you already visited it).   IMPORANT - to include world pass in negative values for start/end angles to ignore so if you want to really ignore something like -45' to 30' make sure you pass in 315,30 instead (use getAngle(..) if you need to)
    
    - **int getAngleWithLowestLightDeltaPct(const int &angle2IgnoreStart, const int &angle2IgnoreEnd)** - Similar to above but gets lowest or darkest change

  - Routines to get the brightest or dimmest light from whereever you are, this doesn't take into account the 'calibration' values, it just finds the brightest/dimmest light based on your surroundings
    - **int getAngleWithBrightestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement)** - Gets the angle with the brighest light; it does not compare to any 'prior' light... it just gives you the angle with the brightest one.  Like other methods you can pass the angles to ignore.. remember all angles are 'world' angles (not related to your current orientation).

    - **int getAngleWithDimmestCurrentLight(const int &angle2IgnoreStart, const int &angle2IgnoreEnd, const int &angleIncrement)** - Similar to above but gives the dimmest light
  
  - Utility methods
    - **int getLightDeltaPctBetween2Values(const int &currentValue, const int &calibrationValue)** - Gets the delta (as a pct 0-100) between two light values... note it only goes to 100% 

    - **int deltaPctHelper(const int &deltaPct, const bool &isPositive)** - Helper method to return the delta pct as an integer

    - **bool numberBetweenRange(const int &theNum, const int &lowValue, const int &highValue)** - Helper it's used to determine if a given number is between a range... it's needed because of angles... say our angle range is from 315' to 45' and we want to see if 30' is between that.  This handles it and will return true; but will return false for something like angle 60.
  
  - Work to triangulate light sources
    - **void setPotentialLightTargets()** - This tries to triangulate your target position to lights... cool work but lights are not good for triangulation :(  The general logic used was (note it handled edge tests but don't describe that below, wanted to just give a gist on how it works):
      ```
        Get the brightest light in world (L1), save your pose (P1)
        Search for a second light (L2) that is (LIGHTANGLERANGETOIGNORE (const) away 
          from that (both to left and right), save your pose (P2)
        Find the midpoint between angles (A1) to L1 and L2
        Travel a distance (LIGHTOPTIMALTRIANGULATION (const)) at angle (A1)
        Get the brightest angle pointing toward light L1, save your pose (P1A)
        You now triangulate where the light is using pose P1 and P1A (not trivial 
          see LocalizationClass::setPointOfIntersection).
        You do the same to triangulate L2
        Knowing the pose of where L1 and L2 are you move to those locations, 
          intent is use wall following and get to the closest point between 
          yourself and the light (unobstructed) and then go to that spot.. 
      ```  
  
  - Output/debugging helping methods
    - **showLightDeltaPctForAngle(const int &theAngle)** - Little helper (mainly for debugging), it will show what the light delta percentages are for a given angle. 

    - **void showLightAttributes(char \*theStr, const LightAttributes &liteAttr, const int &theAngle)** - Good for debugging, msgStr can describe the attributes you're showing, you then give it the attributes and the angle they were taken at (we don't have angle as a light attribute (wouldn't make sense to have it))

    - **void showSampledLightAttributes(const int &theAngle)** - Just a helper method, can use this if you want to show the values that were last sampled

    - **void showCalibrationLightAtAngle(const int &theAngle)** - Show the 'calibration' light attributes for a given angle

    - **void showLightDeltaSum(const lightDeltaSum &amts)** - Show the light delta values; mainly for debugging; this is applicable when you have a LightDeltaSum variable that's been set from calling method setLightDeltaSum.



## LocalizationClass
**Provides attributes/routines related to localization**  fyi a 'Pose' is a structure, it has float 'xPos' and 'yPos', and an int 'angle'
- Localization methods
  - **void setPose(const float &x, const float &y, int const &angle)** -- Sets pose of robot
  - **Pose getPose()** - Returns the robots current pose

  - **void setNewPosition(const float &distance, const int &angleDelta)** - Sets the robots new position, you pass in the distance it's traveled and the angle it traveled from it's current orientation

  - **void setCurrentAngle(const int &angle)** - Sets the robots current angle (be careful resetting robot position)
  - **int getCurrentAngle()** - Gets the robots current angle of orientation

  - **int calculateRealAngleWithAdjustment(const int &angleDelta)** - This returns the current robots pose angle when you add in the delta angle passed... i.e. if robot is at 75' and you call this passing in 30 it'll return 105.  Note this does not change anything about the robots pose

  - **void setCurrentXPosition(const float &x)** - Sets robot's current x position in it's world (be careful with this too)
  - **float getCurrentXPosition()** - Gets the robots x position
  - **int getCurrentXPositionInMM()** - Gets the robots current position in millimeters
  - **int getCurrentXPositionInCM()** - Gets the robots current position in cm (rounded version of the getCurrentXPosition())

  - **float calculateNewX(const float &distance, const int &angleDelta)** - Calculates what the robots new x position would be if they traveled a given distance at a given angle from their current orientation (yes angle here is delta from their orientation)  Note: this doesn't update the robots pose.

  - **void setCurrentYPosition(const float &y)** - Sets robots y position (be careful)
  - **float getCurrentYPosition()** - Gets robots y position
  - **int getCurrentYPositionInMM()** - Gets robots y position in terms of MM
  - **int getCurrentYPositionInCM()** - Gets robots y position in terms of centimeters (rounded version of getCurrentYPosition())

  - **float calculateNewY(const float &distance, const int &angleDelta)** - Calculates what the robots y position would be if traveled a given distance and angle from their current orientation


- Utility methods
  - **int getAngle(int angleInDegrees)** - Returns the 'real' value for an angle... this handles conversion of -angles back to positive ones (i.e -45 to 315) and handles angles like 450 and converts them back to 90
  
  - **float degreesToRadians(const int &degrees)** - Converts degrees to radians
  
  - **int radiansToDegrees(const float &radians)** - Converts radians to degrees
  
  - **byte getQuadrantAngleIsIn(const int &degrees)** - Returns the quadrant a given angle is in
  
  - **convertCoordinateSystemAngle(const int &angle)** Convert angle from one coordinate system to another (i.e. cartesian to spherical)
  
  - **bool closeEnuf(const float &value1, const float &value2, const float &allowedDelta, const boolean &areAngles)** - Checks if two floating point numbers are close enough to one another, the arguments should be obvious (the areAngles is to handle - and + angles correctly )
  
  - **int calculateAngleBetweenPoints(const float &x1, const float &y1, const float &x2, const float &y2)** - Calculates the angle between two points
  
  - **int getMidpointBetweenTwoAngles(const int &angle1, const int &angle2)** - Returns the midpoint angle between two angles  (if you gave it 110 and 90 it'd return 100)
  
  - **int getShortestAngleDeltaToGetToOrientation(const int &targetOrientation)** - Gets shortest angle required to get from the robots current pose to the desired target angle passed in; **Note** the value returned is negative for angles to the left of your current orientation and positive to the right (TODO - handle cartesian changes)
  

  - **float getSlopeOfAngle(const int &degrees)** - Returns the slope of an angle (same as tangent)
  
  - **float getYInterceptForPose(const Pose &thePose)** - Returns the y intercept for a given pose
  - **float distanceBetweenPoses(const Pose &firstPose, const Pose &secondPose)** - Returns the distance between two poses
  - **Pose calculatePose(const Pose &thePos, const int &angleOfMovement, const int &distanceMoved)** - Calculate what the new pose would be, you pass in the starting pose, the angle traveled and distance traveled. **Note** angle traveled should be actual angle, not a delta one
  
- Determine where two poses will intersect
  - **boolean setPointOfIntersection(const Pose &pose1, const Pose &pose2, Pose &pose2Update)** - Takes two poses and update the third pose passed in with the point of intersection.  High level logic
    ```
      Calculate the slope for each pose (tangent)
      Calculate the y intercept for the pose (solve yPos - (slope * xPos))
      You do this for each pose... you know have their line equations (y = mx + b)
      Set the two equations equal and solve x (i.x. x = (pose2 intercept - pose1 intercept) / (pose1 slope - pose2 slope) )
      You have x intercept, use it in one of the equations and solve for y.
      You have the point of intersection but you aren't done yet...
      Calculate the angle for each of your poses to get to the point of intersection... 
        the poses angle has to be the same as the angle you calculated... if not the intersection could be -180' from that pose
    ```
  
Debugging/output methods
  - **void showPose(const Pose &pos2Show)** - Helper method to show a pose
  - **void showLocation()** - Routine to show the robots current pose (this calls showPose with it's own pose)
  - **void writeMsg2Serial(char *theMessage)** - Little helper method to write out a string to the serial line
  - **Pose triangulatePoses(const Pose &firstPose, const Pose &secondPose, const int &angleOfMovement)** - This will triangulate two poses... I would use 'setPointOfIntersection' instead of this, but left it here for future work/reference.


## MovementsClass
**Provides access to movement of the robot**  This handles forward, backard, turning; it also has methods for following wall, adjust to wall etc... if it has to do with changing the robots pose it should be in here :)  This class also has a reference to the other objects (i.e. Localization) so it keeps all the objects in sync.
- Constructor
  - **MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject)** - Constructor, pass in object references the objects needed within the movements class.

- Moving forward/backward
  - **boolean moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance)** - Move us forward a particular distanceToTravel, we'll stop moving forward if we're within the minimum allowed distance to an obstacle (second arg) and the 'checkFrontDistance' flag is true... i.e. with this you can keep moving forward until you see something in front of you that is within some range.

  - **boolean moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance)** - Moves us backward a particular distance, note this is non-blocking so it is intended that you keep calling this method within some loop... it knows the distance it's travelled since it started moving (see other methods) and will stop when it's traveled the distance requested, or that it's moved the minimum distance away from an obstacle (if the third argument is true).

  - **void startMoving(const bool &goForward)** - This starts the robot moving, it sets various attributes time it started moving.. wall state etc.. the parm controls whether you're moving forward (true) or backward (false).  **Note** localization is controlled via this method and the stopMoving method
  
  - **void stopMoving()** - Called to stop the robot from moving... this is the only way you should stop a robot from moving... it sets various attributes that control localization and also determines if it should update it's 'world' coordinates (i.e. the world got bigger as we've traveled into an area that we didn't know existed before)
  
  - **void moveToPose(const Pose &targetPose)** - Move the robot to a particular pose

- Turning methods
  - **void turnLeft(const int &degrees)** - Turns us left by the degrees argument passed in
  - **void turnRight(const int &degrees)** - Turns us right by the degrees passed in
  - **void turnToAngle(const int &theAngle)** - Turn the robot to a particular angle
  - **void turnToZero()** - Moves us back to our origin angle (0')
  - **void turnTo90ClosestDegreeOrientation()** - Will turn us to the closest 90' orientation

- Determining distance traveled 
  - **float getDistanceTraveledSoFar()** -- Returns the distance that we've traveled so far (from the time we started moving)
  - **float getDistanceTraveledForTime(const int &milliseconds)** - Helper method, you give it a time (millis) and it returns the distance traveled in it

- Misc Utility methods
  - **void initMovements()** - Initialization method (called from constructor, it resets your state (i.e. wall opening state, moving state, etc...)
  - **unsigned int getElapsedSinceStartedMoving()** - Helper method to be used within this class... it returns the number of milliseconds between now and the time we started moving
  - **unsigned int getMillisToGetThere(const float &distanceInCM)** - Helper method, it determins how long it will take to travel a given distance.
  - **int getClosest90Angle()** - Helper method to return the closest 90' angle
  - **void setLocalizationPosition(const boolean &inReverse)** - PRIVATE method, used to update the localization pose while robot is moving.

- Determining distance from an obstacle
  - **float getDistanceAtAngle(const int &angle)** - Returns the distance at a given angle, **note** this turns us to that angle; it does not turn us back, cause in most cases we don't want that

- Wall opening/adjusting distance to side wall
  - **void followWall()** - Method to have the robot follow the wall
  - **float adjustDistanceToWall(const float &desiredDistance, const float &currentWallDistance)** - This adjusts our distance to the wall, it stops moving, turns toward the wall and them moves forward or backward to adjust it's length to the wall, it then goes back to it's original pose and resumes it's movements
  - **float wallOpeningDistance(float &distanceToMoveForward, const float &startWallDistance, const float &lastWallDistance)** Returns the distance we've traveled since there has been a wall opening.. the opening has to have the minimum depth (i.e. to fit robot) in order to count as an opening.  This routine is also responsible to call the adjustDistanceToWall (since it's already looking at the wall :))
  
- Debugging/info methods
  - **void showTurnRadius()** - Debugging, shows the turn radius of the robot
  - **void showWallMovements()** - Debugging method to show the wall movements ** TODO NEEDS WORK **

## UltrasonicClass
**Interface for ultrasonic sensor**
  - **NOTE:** The ultrasonic sensor is with +/- the constant ULTRASONIC_TOLERANCE (.3175 cm at time of this writing (1/8 inch))
  
  - **UltrasonicClass::UltrasonicClass()** Constructor
  
  - **void UltrasonicClass::positionServo(int theAngle)** - Position the servo to the angle requested

  - **int UltrasonicClass::getServoAngle()** - Returns the angle the servo is at

  - **float UltrasonicClass::getFreeSpaceInFrontOfGripper(const int &angleOfServo)** - Get the space thats free in front of the grippers, this is probably the most **USEFUL** method; it tells you how far you can travel before you encounter the obstacle.
  
  - **float UltrasonicClass::getFreeSpaceInFrontExcludingGripper(const int &angleOfServo)** -  Returns the free space in front of the robot, this does not include the space the gripper's taking... this is useful when trying to grab an object between the grippers

  - **float UltrasonicClass::getDistanceFromServoAtAngle(const int &angleOfServo)** - Returns the distance from the servo to obstacle (you should probably use methods above instead, but you may want this in certain circumstances)
  
  - **float UltrasonicClass::getDistanceFromCenterOfRobotToObstacle(const int &angleOfServo)** - Returns distance from the center of the robot to the obstacle.  This is especially useful for mapping, you can give it any angle and the value returned is from the robot's pose.
          
  - **float UltrasonicClass::getFreeSpaceOnRight()** - Get the free space between the robot and the wall on the right, this is probably the **SECOND MOST USEFUL**; it's good to see what's on your right side.

  - **float UltrasonicClass::getFreeSpaceOnLeft()** - Get free space between robot and wall on the left, if you're following the left wall this is the 'second most useful' :)

  - **float UltrasonicClass::getSensorTolerance()** - Gets the tolerance of the servo, this is the +/- value from the reading it returns.

  - **void sendObstacleReadingToDetermineWorld(const float &distanceFromServo)** - This method is responsible to let the 'determineWorld' object know that it's seen an obstacle; thought is that every time a sensor reading is taken it gives it to the 'world' obj so that the world obj can keep a map of it's world.
  
  - **void setDetermineWorldObj(DetermineWorldClass &determineWorldObject)** - This method is called with the object reference of the determine world object; it can't be initialized in the constructor as the determine world object gets the ultrasonic obj during its constructor.  This is used so that the ultrasonic object can call the 'recordObstacleFromPoseToLength' method in the determine world object and let it know that it sees an obstacle.

  - **void setLocalizationObj(LocalizationClass &localizationObject)** - Similar to above method, this one is passed in a reference to the localization object.  That's needed to determine pose so that we can pass it to 'sendObstacleReadingToDetermineWorld'

  - **void UltrasonicClass::showUltrasonic(const int &theAngle, const int &theDistance)** - Helper to show ultrasonic value
 
    
    