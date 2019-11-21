#ifndef _INCL_SPARKIC
#define _INCL_SPARKIC

#include <Sparki.h>

#define USE_LCD false
#define USE_CARTESIANSYSTEM false

// Ultrasonic position info
#define ULTRASONIC_FORWARD_OF_CENTER 4.2
#define ULTRASONIC_BEHIND_FRONT 1.3

// The servo reading is from front, the pivot point of the servo is 2.7 cm behind that
#define ULTRASONIC_SERVO_TO_PIVOT 2.7  

// Servo is 1.2 cm forward of the center of the robot when it's facing forward, it's
// at the center location when it's turned right or left
#define ULTRASONIC_SERVO_PIVOT_OFFSET 1.2

// Adjustment for Ultrasonic servo to have it at position 0
// (mine was 5 degrees off)
#define ULTRASONIC_SERVO_ANGLE_ADJUSTMENT 5

// Max effective range of ULTRASONIC 6 feet (~183cm)
#define ULTRASONIC_MAX_RANGE 183.0

// Ultrasonice error rate +- 1/8 inch or .3175 cm
#define ULTRASONIC_TOLERANCE 0.3175

// Min safe approach distance to an obstacle
#define ULTRASONIC_MIN_SAFE_DISTANCE 3.0

// Delta between front and side adjustment, the side reading will be 1.5 cm further then 
// the front reading, but since it only measures in cm I made it 2 cm
#define ULTRASONIC_SIDE_ADJUSTMENT 2

// If on the ultrasonic sensor will send data to the world
#define SEND_ULTRASONIC_READINGS_TO_WORLD true

// Gripper characterisitics
#define GRIPPER_LENGTH 2.6
#define GRIPPER_MAX_WIDTH 4.0

// Velocity for movement
#define VELOCITY_CM_P_SEC 2.78

// Sparki characteristics (length/width), note the axle is in the center
// so the nose length is equal to it's tail :)
#define OVERALL_WIDTH 10.5
#define OVERALL_LENGTH_LESS_GRIPPER 11.28
#define TURN_RADIUS ((OVERALL_LENGTH_LESS_GRIPPER / 2.0) + GRIPPER_LENGTH)

// These values are to be able to determine if an opening in a wall is worth going down... it has
// to have these minimum characteristics
// MINWALLOPENINGDEPTH 
//   Our overall length + 3 * min distance between us an obstacles (3 because we are currently that distance from
//     the wall, if we turn thru the opening we need that space + 2x that for once we go down the opening)
//   actual distance we are from the wall + 1/2 our robot width (this brings us to the all)  (this is about 9 cm)
// MINWALLOPENINGWIDTH is our current width + 2x min safe distance.
#define MINWALLOPENINGDEPTH (OVERALL_LENGTH_LESS_GRIPPER + GRIPPER_LENGTH + (ULTRASONIC_MIN_SAFE_DISTANCE * 3.0)) 
#define MINWALLOPENINGWIDTH (OVERALL_WIDTH + (ULTRASONIC_MIN_SAFE_DISTANCE * 2.0))

// When taking Ultrasonic measurements it's the sample size to use and
// the delay between them
#define ULTRASONIC_SAMPLE_DELAY 5
#define ULTRASONIC_SAMPLE_SIZE 1    // Found out sparki is already sampling and retunging the median value
#define ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT 200
#define ULTRASONIC_RIGHT_ANGLE 85
#define ULTRASONIC_LEFT_ANGLE -85

// Amount to delay after movement
#define DELAY_AFTER_MOVEMENT 100

// Serial device characteristics, speed and amount to delay after communicating
// over serial port
#define SERIAL_SPEED 19200
#define DELAY_FOR_SERIAL_COMM 8

// Amount to delay after serial device is initialized, this gives you time
// to connect monitor/program
#define DELAY_AFTER_SERIAL_STARTUP 5000

// Wall to follow, right (true), left false
#define FOLLOW_RIGHT_WALL true

// Light constants
#define LIGHTSAMPLEANGLE 30  // We can't store 360' of light samples, this is the angle we sample 
#define LIGHTSAMPLESIZE 3  // When taking light reading this is the sample size we use 
#define LIGHTSAMPLEDELAY 50  // Milliseconds to delay between samples
#define LIGHTSAMPLEVALUE2USE 1  // The median value... this is the index position of the median value (sample is sorted)
#define LIGHTCALIBRATIONARRAYSIZE (360/30)  // Degrees / Sample Angle
#define LIGHTANGLERANGETOIGNORE 90 // Degrees to ignore of an existing light source, this is ignore left and right (so it's double)
#define LIGHTSINWORKSPACE 2 // Can't be more than 2 for now
#define LIGHTMINTRIANGULATION 6 // Min cm to be able to triangulate angle
#define LIGHTOPTIMALTRIANGULATION 20  // CM for best triangulation
#define LIGHTDELTAS2ACTON 3  // How many delta's required before we act... i.e. want 3 samples going up before we act... note this is at the
                             // each light source... if the left light has 3 deltas (we'll turn lett by degrees below)  
#define LIGHTDELTAANGLE2TURN 5  // The number of degrees to turn toward the most significant light delta... if center is most significant we don't turn :)


//class SparkiClassCommon {
//  public: static void writeMsg2Serial(char *msg);
//};

#endif