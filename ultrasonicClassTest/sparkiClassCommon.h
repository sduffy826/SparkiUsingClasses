#ifndef _INCL_SPARKIC
#define _INCL_SPARKIC

#define USE_LCD false

// Ultrasonic position info
#define ULTRASONIC_FORWARD_OF_CENTER 4.2
#define ULTRASONIC_BEHIND_FRONT 1.3

// Adjustment for Ultrasonic servo to have it at position 0
// (mine was 5 degrees off)
#define ULTRASONIC_SERVO_ANGLE_ADJUSTMENT 5

// Max effective range of ULTRASONIC 6 feet (~183cm)
#define ULTRASONIC_MAX_RANGE 183

// Min safe approach distance
#define ULTRASONIC_MIN_SAFE_DISTANCE 6

// Gripper characterisitics
#define GRIPPER_LENGTH 2.6
#define GRIPPER_MAX_WIDTH 4.0

// Velocity for movement
#define VELOCITY_CM_P_SEC 2.78

// Sparki characteristics (length/width), note the axle is in the center
// so the nose length is equal to it's tail :)
#define OVERALL_WIDTH 10.5
#define OVERALL_LENGTH_LESS_GRIPPER 11.28
#define TURN_RADIUS (OVERALL_LENGTH_LESS_GRIPPER / 2.0) + GRIPPER_LENGTH

// When taking Ultrasonic measurements it's the sample size to use and
// the delay between them
#define ULTRASONIC_SAMPLE_DELAY 5
#define ULTRASONIC_SAMPLE_SIZE 3
#define ULTRASONIC_DELAY_AFTER_SERVO_MOVEMENT 200
#define ULTRASONIC_RIGHT_ANGLE 85
#define ULTRASONIC_LEFT_ANGLE -85

// Amount to delay after movement
#define DELAY_AFTER_MOVEMENT 100

// Serial device characteristics, speed and amount to delay after communicating
// over serial port
#define SERIAL_SPEED 9600
#define DELAY_FOR_SERIAL_COMM 8

// Amount to delay after serial device is initialized, this gives you time
// to connect monitor/program
#define DELAY_AFTER_SERIAL_STARTUP 10000

// Wall to follow, right (true), left false
#define FOLLOW_RIGHT_WALL true

#endif