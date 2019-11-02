
## NOTE
- I tried to make common code (include files) but couldn't get arduino ide to recognize the path to different directories.  It appears the code has to be in the directory that wants to use it.  What I did is have the code for an area in a .h & .cpp file in the directory for that function.  I named the directory with a 'Test' suffix (i.e. directory localizationClassTest is the directory for the test program (.ino) and where it's localizationClass.h and localizationClass.cpp files are.  To use that code in a different area (i.e. determineWorldTest) you copy the .h & .cpp files into the determineWorldTest directory.  I created a **'syncClassFiles.sh'** script in git/root path, when you run it it copies the required files into the directories that need them.  When you write a new area just update it to include the .h/.cpp files you want to use.  MAKE SURE you keep it up to date, and only update the .h/.cpp file in the correct location.  Also, you need to close/open the folder after you've run the shell script (the ide doesn't know if underlying file was updated).
- Memory is a concern, you'll probably want to uncomment NO_LCD, NO_ACCEL and NO_MAG in the Sparki.h file.  You should also set USE_LCD to false in sparkiCommon.h... if that's false then (most of) the code will output values to the serial port instead of the lcd screen; if it's true it'll try to write the debugging/log info to the lcd

- Here's a brief overview of includes you should be interested in, look at the appropriate .cpp's if want to see implementation
  - **sparkiClassCommon.h** has common definitions and global variable definitions; you should be able to control a lot of functionality by just tweaking the 'defines' listed here
  - **determineWorldClass.h** code for managing the robots world 
  - **localizationClass.h** code related to localization (more below)
  - **movementsClass.h** code to make the robot move (forward, backward, turning...)
  - **ultrasonicClass.h** code fo interfacing with the ultrasonic sensor

## To Do's
- Check legend abbreviations
- Document the lights class
- Create a 'wander world' or explore world routine
- Cleanup/refine the routine to follow wall
- Couldn't get light triangulation to work, try it with flashlights that can give a more pinpoint/direct light source
- Test light logic, to find light may want to use shadow logic; see if light is affected by other sensors, magnetometer...
- Revisit using accelerometer to detect collisions... at first I couldn't get working but try again
- Have a battery life state
- Remember can use struct and define var as int : 3; to mean use three bits 0-7 values; have to clean up some code

## Code for testing/calibration (most of this is in the ../sparki directory, and the sparki repo)
- **movementTest**: This has code used to test movements; I used this to calculate how fast sparki is (2.78cm/sec); to do this you can use the moveForward function, it'll show the elapsed time to move that distance.  I found that sparki was off; it actually moved further than requested.  See the sparkiMovementAnalysis spreadsheet in the sparki_python repo for calculations I did.
- **sensorTest**: Code for testing sensors; it outputs a bunch of values from the sensors... have the python code reading the serial port so that it can provide mean, median, mode, std deviation for the various sensors.
- **PlaygroundToTest**: Just stub for testing various functions, a play area :)
- **wallAdjustmentTest** Code I was using to try and adjust when the sparki is moving away from a wall at an angle
  - Note, methods are: 
    - To wiggle toward the wall (gripper should be wide), and continue to do that, what happens is one of the grippers hit the wall so it can't move forward in that direction, and eventually the robot will get in a position where he's 90' to the wall; you then just backup, and turn 90'... you're now parallel to the wall
    - You need to have enough distance to do this, so keep track of how long the wall has been next to you... when you see a state change in distance you log where you are... you continue to move forward till it hits the next increment; you now have a distance traveled in x direction, and delta in the y direction; you can calculate the angle that you are offset by.... you then backup till you're at desired distance to the wall (note may want to go little more to account for +- of the sensor), you then stop and turn using the angle you just calculated.
  
## LEGEND Serial communications
**Memory** is critical and strings take up quite a bit of memory, I used the abbreviations below when outputting to the serial port.  
- **DW** Define world, world dimensions (x,y)
- **DP** Define world, current position (x,y)
- **LO** Localization, x position, y position, &lt; current angle
- **LA** Light angle, has description of what it is, it's angle, brightness for left, center and right light sensor
- **LB** Light brightness delta, shows the angle that has largest increase in brightness over the sample; it will ignore the quadrant if requested
- **LCD** Light sample, it shows the angle and the light brightness for the center light
- **LBD** Light brightness delta for all angles, this is only shown if DEBUGLIGHT is on
- **LBIR** Light brightness ignore angle range (only when DEBUGLIGHT is on)
- **LBI** Light brightness, angle that is ignored in calculation (when looking for second light we ignore the area that we found the first one) (also only when DEBUGLIGHT is on)
- **US** Ultrasonic sensor

# Code/Class Definitions 
I listed the classes alphabetically except for the first one... that has the constants so you should know about that one first.

## SparkiClass
- **SparkiCommon**: this has the .h file to be included in code; it defines constants for the sparki like: it's speed, dimensions, etc... you should put constants here (used define instead of const) to save memory.  When you update this run ./syncClassFiles.sh 

## DetermineWorldClass
**Controls/has information about the robots world** 
- **DetermineWorldClass()** - 'default' constructor, required by arduino, but you should initialize object with references to required objects (see next constructor)
- **DetermineWorldClass(UltrasonicClass &ultrasonicObj, LocalizationClass &localizationObj)** - Constructor where you pass references to required objects needed within this object.
- **WorldCoord getWorldCoordinates()** - Gets your world coordinates
- **void checkWorldCoordinates()** - Call this periodically to check and adjust world coordinates based on current position; i.e. when you first called getWorldCoordinates some things may not have been visible; if you call this method periodically (i.e. when you stop) you can adjust the 'world' coordinates.
- **void showWorld()** - This is just a helper to show your world... mainly for debugging.
- **void calculateRectangularCoordinates()** - Calculates your 'world' space, right now it takes the direction at 90' angles and records them; it also calls localization methods that set's your current position within that world.


## LightsClass
**Provides attributes and routines related to the light sensors** Note it's common to call 'sampleWorldLights()' then turn the lights on (or move) and call 'calculateWorldDeltaPcts()' which will calculate the deltas.  You can then use a method like 'getAngleWithHighestLightDelta(const int &angle2IgnoreStart, const int &angle2IgnoreEnd)' to find the angle with brighest light (read more below).
```
// struct for light attributes... since we only have values from 0-1000 we use 10 bits... the flag1/flag2 are for future and to make the size of this align on the proper boundary.
struct LightAttributes {
  unsigned int lightLeft : 10;
  unsigned int lightCenter : 10;
  unsigned int lightRight : 10;
  unsigned int flag1 : 1;
  unsigned int flag2 : 1;
};

// Struct to hold the pct change 0-100 between the 'world sampled values' (lightCalibration array).  NOTE: we are very memory conscious that's why were doing 'bit' level definitions... if a percentage is positive the associated *SignBit will be on (1), if it's negative it will be off.  Note: use the deltaPctHelper... you pass it the *Pct value and the *SignBit... it'll return what the percentage for you.
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

- **public methods**
  - Routines to sample (calibrate) lights and then determine current lights and updates the pct differences for each angle
    - **void sampleWorldLights()** - Takes a sample of the 'world lights' and stores values in the lightCalibration array (for respective angles)
    - **void calculateWorldLightDeltas()** - This updates the lightDeltaPcts array with the difference between the 'world' lights now and the original values taken, note, you should have called 'sampleWorldLights()' before using this one... 
    
  - Routine to get current light attributes (where you currently are and your current pose only)
    - **LightAttributes getLightAttributesAtCurrentPose()** - Returns the light attributes at the current pose you are at, this takes a sampling (calls setLightSampleAttributesAtCurrentPose) and returns those values to the caller.
  
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
  
  - Try to triangulate light sources
    - **void setPotentialLightTargets()** - This tries to triangulate your target position to lights... cool work but lights are not good for triangulation :(  The general logic used was (not handled edge tests... but this will give gist):
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
          intent is use wall following and get to the closes point between 
          yourself and the light (unobstructed) and then go to that spot.. 
      ```  
  
  - Output/debugging helping methods
    - **showLightDeltaPctForAngle(const int &theAngle)** - Little helper (mainly for debugging), it will show what the light delta percentages are for a given angle.  
    - **void showLightAttributes(char \*theStr, const LightAttributes &liteAttr, const int &theAngle)** - Good for debugging, msgStr can describe the attributes you're showing, you then give it the attributes and the angle they were taken at (we don't have angle as a light attribute (wouldn't make sense to have it))
    - **void showSampledLightAttributes(const int &theAngle)** - Just a helper method, can use this if you want to show the values that were last sampled
    - **void LightsClass::showCalibrationLightAtAngle(const int &theAngle)** - Show the 'calibration' light attributes for a given angle



## LocalizationClass
**Provides attributes/routines related to localization**  fyi a 'Pose' is a structure, it has float 'xPos' and 'yPos', and an int 'angle'
- **convertCoordinateSystemAngle(const int &angle)** Convert angle from one coordinate system to another (i.e. cartesian to spherical)
- **void setPose(const float &x, const float &y, int const &angle)** -- Sets pose of robot
- **Pose getPose()** - Returns the robots current pose
- **void setCurrentAngle(const int &angle)** - Sets the robots current angle (be careful resetting robot position)
- **int getCurrentAngle()** - Gets the robots current angle of orientation
- **void setCurrentXPosition(const float &x)** - Sets robot's current x position in it's world (be careful with this too)
- **float getCurrentXPosition()** - Gets the robots x position
- **int getCurrentXPositionInMM()** - Gets the robots current position in millimeters
- **int getCurrentXPositionInCM()** - Gets the robots current position in cm (rounded version of the getCurrentXPosition())
- **void setCurrentYPosition(const float &y)** - Sets robots y position (be careful)
- **float getCurrentYPosition()** - Gets robots y position
- **int getCurrentYPositionInMM()** - Gets robots y position in terms of MM
- **int getCurrentYPositionInCM()** - Gets robots y position in terms of centimeters (rounded version of getCurrentYPosition())
- **bool closeEnuf(const float &value1, const float &value2, const float &allowedDelta, const boolean &areAngles)** - Checks if two floating point numbers are close enough to one another, the arguments should be obvious (the areAngles is to handle - and + angles correctly )
- **int calculateAngleBetweenPoints(const float &x1, const float &y1, const float &x2, const float &y2)** - Calculates the angle between two points
- **float getSlopeOfAngle(const int &degrees)** - Returns the slope of an angle (same as tangent)
- **float getYInterceptForPose(const Pose &thePose)** - Returns the y intercept for a given pose
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
- **float degreesToRadians(const int &degrees)** - Converts degrees to radians
- **int radiansToDegrees(const float &radians)** - Converts radians to degrees
- **byte getQuadrantAngleIsIn(const int &degrees)** - Returns the quadrant a given angle is in
- **int getAngle(int angleInDegrees)** - Returns the 'real' value for an angle... this handles conversion of - angles back to positive ones and handles angles like 450 and convers them back to 90
- **int calculateRealAngleWithAdjustment(const int &angleDelta)** - This returns the current robots pose angle when you add in the delta angle passed... i.e. if robot is at 75' and you call this passing in 30 it'll return 105
- **int getMidpointBetweenTwoAngles(const int &angle1, const int &angle2)** - Returns the midpoint between two angles
- **float calculateNewX(const float &distance, const int &angleDelta)** - Calculates what the robots new x position would be if they traveled a given distance at a given angle from their current orientation (yes angle here is delta from their orientation)
- **float calculateNewY(const float &distance, const int &angleDelta)** - Calculates what the robots y position would be if traveled a given distance and angle from their current orientation
- **void setNewPosition(const float &distance, const int &angleDelta)** - Sets the robots new position, you pass in the distance it's traveled and the angle it traveled from it's current orientation
- **int getShortestAngleDeltaToGetToOrientation(const int &targetOrientation)** - Gets shortest angle required to get from the robots current pose to the desired target angle passed in; **Note** the value returned is negative for angles to the left of your current orientation and positive to the right (TODO - handle cartesian changes)
- **void showPose(const Pose &pos2Show)** - Helper method to show a pose
- **void showLocation()** - Routine to show the robots current pose (this calls showPose with it's own pose)
- **void writeMsg2Serial(char *theMessage)** - Little helper method to write out a string to the serial line
- **float distanceBetweenPoses(const Pose &firstPose, const Pose &secondPose)** - Returns the distance between two poses
- **Pose calculatePose(const Pose &thePos, const int &angleOfMovement, const int &distanceMoved)** - Calculate what the new pose would be, you pass in the starting pose, the angle traveled and distance traveled. **Note** angle traveled should be actual angle, not a delta one
- **Pose triangulatePoses(const Pose &firstPose, const Pose &secondPose, const int &angleOfMovement)** - This will triangulate two poses... I would use 'setPointOfIntersection' instead of this, but left it here for future work/reference.


## MovementsClass
**Provides access to movement of the robot**
- **MovementsClass(UltrasonicClass &ultrasonicObject, LocalizationClass &localizationObject, DetermineWorldClass &determineWorldObject)** - Constructor, pass in object references the objects needed within the movements class.
- **void initMovements()** - Initialization method (called from constructor, it resets your state (i.e. wall opening state, moving state, etc...)
- **unsigned int getElapsed()** - Helper method to be used within this class... it returns the number of milliseconds between now and the time we started moving
- **unsigned int getMillisToGetThere(const float &distanceInCM)** - Helper method, it determins how long it will take to travel a given distance.
- **void startMoving(const bool &goForward)** - This starts the robot moving, it sets various attributes time it started moving.. wall state etc.. the parm controls whether you're moving forward (true) or backward (false).  **Note** localization is controlled via this method and the stopMoving method
- **float getDistanceTraveledForTime(const int &milliseconds)** - Helper method, you give it a time (millis) and it returns the distance traveled in it
- **float getDistanceTraveledSoFar()** -- Returns the distance that we've traveled so far (from the time we started moving)
- **void stopMoving()** - Called to stop the robot from moving... this is the only way you should stop a robot from moving... it sets various attributes that control localization and also determines if it should update it's 'world' coordinates (i.e. the world got bigger as we've traveled into an area that we didn't know existed before)
- **boolean moveBackward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance)** - Moves us backward a particular distance, note this is non-blocking so it is intended that you keep calling this method within some loop... it knows the distance it's travelled since it started moving (see other methods) and will stop when it's traveled the distance requested, or that it's moved the minimum distance away from an obstacle (if the third argument is true).
- **boolean moveForward(const float &distanceToTravel, const float &minAllowedDistanceToObstacle, const bool &checkFrontDistance)** - Move us forward a particular distanceToTravel, we'll stop moving forward if we're within the minimum allowed distance to an obstacle (second arg) and the 'checkFrontDistance' flag is true... i.e. with this you can keep moving forward until you see something in front of you that is within some range.
- **void turnLeft(const int &degrees)** - Turns us left by the degrees argument passed in
- **void turnRight(const int &degrees)** - Turns us right by the degrees passed in
- **int getClosest90Angle()** - Helper method to return the closes 90' angle
- **void turnToAngle(const int &theAngle)** - Turn the robot to a particular angle
- **int getDistanceAtAngle(const int &angle)** - Returns the distance at a given angle, **note** this turns us to that angle; it does not turn us back, cause in most cases we don't want that
- **void turnToZero()** - Moves us back to our origin angle (0')
- **void turnTo90ClosestDegreeOrientation()** - Will turn us to the closes 90' orientation
- **int adjustDistanceToWall(const int &desiredDistance, const int &currentWallDistance)** - This adjusts our distance to the wall
- **float wallOpeningDistance(int &distanceToMoveForward, const int &startWallDistance, const int &lastWallDistance)** Returns the distance we've traveled since there has been a wall opening.. the opening has to have the minimum depth (i.e. to fit robot) in order to count as an opening
- **void followWall()** - Method to have the robot follow the wall
- **void moveToPose(const Pose &targetPose)** - Move the robot to a particular pose
- **void showTurnRadius()** - Debugging, shows the turn radius of the robot
- **void showWallMovements()** - Debugging method to show the wall movements ** TODO NEEDS WORK **

## UltrasonicClass
**Interface for ultrasonic sensor**
  - **NOTE:** The ultrasonic sensor is with +/- the constant ULTRASONIC_TOLERANCE (.3175 cm at time of this writing (1/8 inch))
  - **UltrasonicClass::UltrasonicClass()** Constructor
  - **void UltrasonicClass::positionServo(int theAngle)** -Position the servo to the angle requested
  - **int UltrasonicClass::distanceAtAngle(const int &angleOfServo)** - Get distance at a particular angle
  - **int UltrasonicClass::distanceRight()** - Get distance at right angle
  - **int UltrasonicClass::distanceLeft()** - Get distance at left angle
  - **float UltrasonicClass::getActualBodyDistanceFromFront(const int &reportedDistance)** - Get the actual open space between front of robot and the object it reported
  - **float UltrasonicClass::getActualGripperDistanceFromFront(const int &reportedDistance)** - Get distance between gripper tip and the object.
  - **float UltrasonicClass::getActualCenterOfBodyDistanceFromFront(const int &reportedDistance)** - Get distance between the center of robot and the object it reported in front of it
  - **float UltrasonicClass::getActualBodyDistanceFromSide(const int &reportedSideDistance)** - Get the distance between side of robot and the wall it's next to 
  - **float UltrasonicClass::getActualCenterOfBodyDistanceFromSide(const int &reportedSideDistance)** - Get distance between the center of robot and the object it reported on it's side.
  - **float UltrasonicClass::getAdjustedUltrasonicReadingAfterRotation(const int &reportedDistance, const bool &nowAtZeroAngle)** - This method should only be used when you want to take a reading and calculate what your distance to the wall would be at your new orientation... this is mainly used when turning 90' and want to proceed thru an opening (you can't read wall distance cause your past it).
  - **void UltrasonicClass::showUltrasonic(const int &theAngle, const int &theDistance)** - Helper to show ultrasonic value
 