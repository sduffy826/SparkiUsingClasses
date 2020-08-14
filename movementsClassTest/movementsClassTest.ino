#include "movementsClass.h"

byte counter;

void setup() {
  // put your setup code here, to run once:
  #if USE_LCD 
    sparki.clearLCD();
    delay(500);
  #else
    Serial.begin(SERIAL_SPEED);
    sparki.beep();
    delay(DELAY_AFTER_SERIAL_STARTUP);  
    sparki.beep();
  #endif
  counter = 0;
}

void loop() {
  // Just want to test once :)
  if (counter == 0) {
      
    counter++;
    
    UltrasonicClass ultrasonicObj;
    LocalizationClass localizationObj;
    DetermineWorldClass determineWorldObj(ultrasonicObj, localizationObj);
    //determineWorldObj.adjustWorldCoordinate(100.0,200.0);  // Just for testing :)
    
    MovementsClass movementObj(ultrasonicObj, localizationObj, determineWorldObj);

    if (true == false) {
      movementObj.initMovements();

      for (int i = 0; i < 20; i++) {
        int theDistance = ultrasonicObj.getFreeSpaceOnRight();
        Serial.print("Distance from right distance: ");
        Serial.println(theDistance);
        delay(500);
      }
    }


    if (true == false) {
      movementObj.initMovements();
      movementObj.turnRight(90); 
      int theDistance = ultrasonicObj.getFreeSpaceInFrontOfGripper(0);
      while (movementObj.moveForward(theDistance-ULTRASONIC_MIN_SAFE_DISTANCE, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      
      theDistance = ultrasonicObj.getFreeSpaceInFrontOfGripper(0);
      Serial.print("Forward distance: ");
      Serial.println(theDistance);
      
      movementObj.turnLeft(90);
      theDistance = ultrasonicObj.getFreeSpaceOnRight();
      Serial.print("Distance from right distance: ");
      Serial.println(theDistance);
    }    

    // Random exploration... this will randomly move about the world and take readings.
    // The intent is to have the python program 'readSerialObstacles.py' running while this
    // is, it'll take these readings and write them to a file.  You can then use the
    // python program 'MapWorldTester.py' to graphically display it (it uses the MapWorldClass.py 
    // program).
    // fyi, this is really for demo, and the python code should be in /seanduff/VS_Code/sparki
    if (true == true) {
      localizationObj.setPose(0.0,0.0,0);

      #define MINSERVO -80
      #define MAXSERVO 80
      int currServoAngle = 0;
      int increment = 10;
      float distance2Obstacle = ultrasonicObj.getDistanceFromServoAtAngle(currServoAngle);
      for (int i = 0; i < 20; i++) {
        int angle2Turn2 = random(0,360);  // gives values 0->359
        movementObj.turnToAngle(angle2Turn2);
        distance2Obstacle = ultrasonicObj.getDistanceFromServoAtAngle(currServoAngle);
        while (distance2Obstacle > 25.0) {
          movementObj.moveForward(999.0,0,false);
          increment = (currServoAngle == MINSERVO ? 10 : (currServoAngle == MAXSERVO ? -10 : increment));
          currServoAngle += increment;
          distance2Obstacle = ultrasonicObj.getDistanceFromServoAtAngle(currServoAngle);
          Serial.print(currServoAngle);
          Serial.print(",");
          Serial.println(distance2Obstacle);
        }
      }
    }
    
    if (true == false) {
      // Test moving, we save original spot, move some distance away and then return to it
      movementObj.initMovements();
      localizationObj.setPose(13.2,7.6,30.0);
      Pose originalSpot = localizationObj.getPose();
      
      localizationObj.showPose(originalSpot);
      
      // Move 5cm, turn, move 2 more, then return
      // while (movementObj.moveBackward(15.0, ULTRASONIC_MIN_SAFE_DISTANCE, false)) {
      while  (movementObj.moveForward(15.0, ULTRASONIC_MIN_SAFE_DISTANCE, true)) {
        // Want to prove that the localization data is changing while moving
        localizationObj.showPose(localizationObj.getPose());
        delay(10);
      }
      sparki.beep();
      localizationObj.showPose(localizationObj.getPose());
      
      movementObj.turnLeft(90);
      while (movementObj.moveForward(7.0, ULTRASONIC_MIN_SAFE_DISTANCE, true));
      
      localizationObj.showLocation();
      
      movementObj.moveToPose(originalSpot);
      localizationObj.showLocation();
      
    }
    
    if (true == false) {
      // routine below has the logic to calculate our rectangular world coordinates
      movementObj.initMovements();
      movementObj.followWall();
    }

    if (true == false) {
      movementObj.showTurnRadius();
    }

    if (true == false) {
      movementObj.turnRight(90);
    }

    if (true == false) {
      movementObj.turnRight(134);
      delay(2000);
      movementObj.turnToClosest90DegreeOrientation();
      delay(2000);
      movementObj.turnToZero();
    }

    
  }
}
