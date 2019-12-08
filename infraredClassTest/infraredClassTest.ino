#include "infraredClass.h"

byte counter;
/* ==========================================================================  
 * *****  N O T E ****
 * 
 * When we send the 'pose' values to the python program they are the pose
 * of the sensor.  The python program converts them to the pose of the
 * robot (4cm behind), when it tells us to go to a position it is telling
 * use the pose of the robot, not the sensor pose (which wouldn't make sense
 * since that's not on the line)
 * ==========================================================================
 */
void setup() {
  // put your setup code here, to run once:
  #if USE_LCD 
    sparki.clearLCD();
    delay(500);
  #else
    Serial.begin(19200); //SERIAL_SPEED);
    sparki.beep();
    delay(5);
    sparki.beep();
    delay(DELAY_AFTER_SERIAL_STARTUP);  
   // Serial.setTimeout(120000);  // Set serial timeout to 2 minutes 
   
    // Clear anything on the serial port
    if (Serial.available() > 0)  {
      while (Serial.available() > 0) {
        Serial.read();         
      }
      delay(DELAY_FOR_SERIAL_COMM);
    }
    sparki.beep();
  #endif
  counter = 0;
}

char handShakeWithComputer(LocalizationClass &locObj) {
  LocalizationClass *locPtr = &locObj;
  //locPtr->writeMsg2Serial("IR,HNDSHK");
  Serial.println(F("IR,HNDSHK"));
  char rtnChar = ' ';
  char lastChar = ' ';
  byte iteration = 0;
  while ((iteration < 20) && (rtnChar == ' ')) {
    if (Serial.available() > 0) {
      rtnChar = (char)Serial.read();
      if (lastChar != '$') {
        lastChar = rtnChar;
        rtnChar  = ' ';
      }
    }
    else 
      delay(500);
    iteration++;
  }
  delay(20);
  // Clear port
  while (Serial.available() > 0) {
    Serial.read(); 
  }
    
  return rtnChar;      
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void loop() {
  // Just want to test once :)
  if (counter == 0) {
    counter++;

    // For testing here we want pointers for the code below, test it with 
    UltrasonicClass ultrasonicObject;
    UltrasonicClass *ultrasonicObj;
    ultrasonicObj = &ultrasonicObject;
    
    LocalizationClass localizationObject;
    LocalizationClass *localizationObj;
    localizationObj = &localizationObject;
    
    localizationObj->setPose(0.0, 0.0, 0);

    DetermineWorldClass determineWorldObject(ultrasonicObject, localizationObject);
    DetermineWorldClass *determineWorldObj;
    determineWorldObj = &determineWorldObject;
    
    MovementsClass movementsObject(ultrasonicObject, localizationObject, determineWorldObject);
    MovementsClass *movementsObj;
    movementsObj = &movementsObject;
    
    InfraredClass infraredObject(localizationObject, movementsObject);
    InfraredClass *infraredObj;
    infraredObj = &infraredObject;
    
    // We don't need to calculate the world... we'll update it as we move
    // thru the world

    #define TESTOBSTACLE false
    #if TESTOBSTACLE
      for (int i = 0; i < 10; i++) {
        Serial.print(F("getFreeSpaceInFrontExcludingGripper(0)"));
        Serial.println(ultrasonicObj->getFreeSpaceInFrontExcludingGripper(0));
        delay(200);
        Serial.print(F("getDistanceFromCenterOfRobotToObstacle(0)"));
        Serial.println(ultrasonicObj->getDistanceFromCenterOfRobotToObstacle(0));
        delay(200);
        Serial.print(F("getFreeSpaceInFrontOfGripper(0)"));
        Serial.println(ultrasonicObj->getFreeSpaceInFrontOfGripper(0));
        delay(200);
      }
    #endif


    // ----------------------------------------------------------------------------------------
    // Preliminary navigation work - follow tape
    // ----------------------------------------------------------------------------------------
    #define TESTMAZE true
    #if TESTMAZE
      QueueArray<InfraredInstructions> queueOfInstructions;
      InfraredInstructions currentInstruction;
    
      delay(3000);
      sparki.beep();

      Pose poseOfSensor = infraredObj->getPoseOfCenterSensor();
      
      InfraredAttributes currAttributes, lastAttributes;
      
      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      int baseEdgeLeft = infraredObj->getBaseAttributes().edgeLeft;  // Save edge reading, we'll use this to check for intersections
      if (DEBUGINFRARED) infraredObj->showInfraredAttributes("Base",infraredObj->getBaseAttributes(),poseOfSensor,false);
      
      //int numStateChanges = 0;
      bool done = false;
      #if DEBUGINFRARED
        unsigned long baseTimer = millis();
        unsigned long currTimer;
      #endif

      bool waitForInstructions = false;  
      //bool setLastAttributes = true;
      bool check4Obstacle      = false;
      bool isMoving            = false;
      bool adjustedLine        = false;
      char lastInstructionMode = ' ';
      bool resetStartPose      = false;

      unsigned int startDistanceInMM = 0;
      float currDistanceTraveled;
   
      float distanceToTravel = INFRARED_MAX_DISTANCE_TO_TRAVEL;  // Just some long distance for the first time  

      currentInstruction.instruction = handShakeWithComputer(localizationObject);  // The python program will tell us the mode we're in
      //currentInstruction.instruction = handShakeWithComputer();  // The python program will tell us the mode we're in
      if (currentInstruction.instruction == ' ') {
        distanceToTravel = 0.0;
        done             = true;
        if (DEBUGINFRARED) Serial.print("!HndShk");
      }
      else
        if (currentInstruction.instruction == GOAL_MODE) // If we are starting in goal mode then we need to resetStartPose
          resetStartPose = true;
      
      // the while is basically saying move forward a distance, the only time we check the ultrasonic distance is if we are in 'goal mode'
      //   changed for memory... while ( (movementsObj->moveForward(distanceToTravel,ULTRASONIC_MIN_SAFE_DISTANCE,
      //                                    (currentInstruction.instruction==GOAL_MODE)) == true) || (done == false) ) {
      while ( (movementsObj->moveForward(distanceToTravel,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) || (done == false) ) {  

        if (WRITEMAPDATA == true) localizationObj->showLocation("IP");
        
        currDistanceTraveled = movementsObj->getDistanceTraveledSoFar();
        //   Serial.print("mem"); Serial.println(freeRam());   this didn't seem to be accurate... left for reference
        
        // Get the attributes at the current pose, changed so pass in variable to be modified to the getter... it saves memory
        infraredObj->getInfraredAttributesAtSensorPose(currAttributes);
        
        isMoving = movementsObj->isMoving();
        if (DEBUGINFRARED && (isMoving == false)) Serial.println("isMovingIsFalse");
        
        // See if the current instruction is different from the last one... if so send a message down the serial port
        //   to let the program know that we started a new instruction
        if (lastInstructionMode != currentInstruction.instruction) {
          lastAttributes = currAttributes;  // Don't trigger state change when we just got a new instruction
          
          // Tell python program that we are at start of a new instruction, we do the 'end of instruction' down below 
          // because when it changes we're always at 'waitForInstruction'

          // Note because of memory limitations I'm not using the localizationObj to write to the serial port (ugh), it's strange
          // changing the other reference to user Serial.print actually increased memory
          //   localizationObj->writeMsg2Serial("IR,INSSTART,",false);
          //   localizationObj->writeChar2Serial(currentInstruction.instruction,false);
          //   localizationObj->writeChar2Serial(',',false);
          
          Serial.print(F("IR,INSSTART,"));
          Serial.print(currentInstruction.instruction);
          Serial.print(',');
          localizationObj->showPose(poseOfSensor);
          lastInstructionMode = currentInstruction.instruction;
          adjustedLine        = false;
          startDistanceInMM   = (int)(currDistanceTraveled*10);          
        }
        else
          if ((int)currDistanceTraveled*10 < startDistanceInMM) // Ensure that we don't generate - number which causes an overflow
            startDistanceInMM = (int)currDistanceTraveled*10;

        if (DEBUGINFRARED) {     
          Serial.print((int)(currDistanceTraveled*10));
          Serial.print(",");
          Serial.println(startDistanceInMM);
        }

        // If out of memory then you can disable the line adjusting by turning flag on below
        // The logic for adjusting to tape is: if and edge sensor is one or we've traveled a given distance check
        //   that we're on the center of the tape... note this doesn't work if intersections are too close to eachother..
        //   if memory wasn't a concern I'd check that we traveled a minimum distance before checking the edge but 
        //   there isn't enuf memory to add that now... if find a way to save more memory then add that here
        //adjustedLine = true;
        /* COMMENTED OUT this block... thinking if change constants to be 5 cm we should be ok... when we make a turn
         *  the robot is 2cm in front of the tape... we only need 7 cm of space to check if on a line... test this
         *  out and see if it resolves issue
        if (adjustedLine == false) {
          if ( (infraredObj->onIntersection(baseEdgeLeft) == true) ||
               ( ((int)(currDistanceTraveled*10) - startDistanceInMM) > (INFRARED_SPACE_BEFORE_LINE_CHECK*10) ) ) {
            distanceToTravel -= movementsObj->getDistanceTraveledSoFar();     // Get remaining distance for when we start moving
            movementsObj->stopMoving();                                       // Stop moving
            adjustedLine = infraredObj->adjustToTape();
            continue;  // Go back to the start of the loop, this will make us start moving again
          }
        }
        */

        int deltaMM = (int)(currDistanceTraveled*10) - startDistanceInMM;
        // We need to recheck if we go a long distance :)
        if ((adjustedLine == true) && ( deltaMM > INFRARED_SPACE_RECHECK_LINE_MM)) 
          adjustedLine = false;
       
        //if ((adjustedLine == false) && ( ((int)(currDistanceTraveled*10) - startDistanceInMM) > (INFRARED_SPACE_BEFORE_LINE_CHECK*10))) {
        if ((adjustedLine == false) && (deltaMM > INFRARED_SPACE_BEFORE_LINE_CHECK_MM)) {
          if (DEBUGINFRARED) { Serial.println("*A*"); localizationObj->showPose(poseOfSensor); }
          Serial.println(F("AP"));  // Ajust pose... tell python what we're doing
          Serial.flush();
          
          distanceToTravel -= movementsObj->getDistanceTraveledSoFar();     // Get remaining distance for when we start moving
          movementsObj->stopMoving();                                       // Stop moving
          adjustedLine = infraredObj->adjustToTape();
          continue;  // Go back to the start of the loop, this will make us start moving again
        }
        
        // This is just for debugging       
        #if DEBUGINFRARED
          currTimer = millis();
          Serial.print("timeToMeas:");
          Serial.println((float)(currTimer-baseTimer)/1000.0);
          baseTimer = currTimer;
        #endif

        // We continue moving till the state changes or we're done moving
        if (infraredObj->stateChanged(currAttributes, lastAttributes) || (isMoving==false)) {
          lastAttributes = currAttributes;
          if (DEBUGINFRARED) localizationObj->writeMsg2Serial("StateChange");

          waitForInstructions = false;  // We want the computer to tell us where to go next
          check4Obstacle      = false;  // We only check this when at an exit

          // In this block we check the STATEs
          if (currAttributes.atExit) {  // We found an exit, stop moving and wait for instructions on where to go
            waitForInstructions = true;
            check4Obstacle      = true;
          }
          else if (currAttributes.startLeftPath || currAttributes.startRightPath) {  // Start of path that's to left or right
            startDistanceInMM = (int)(currDistanceTraveled*10);
            // *********** CHANGE SO THAT IT ENSURES IT'S AT INTERSECTION AND NOT DRIFTED INTO LINE.... THINKING
            // YOU COULD BACKUP A LITTLE, TURN LEFT TILL U SEE LINE KEEP TRACK OF ANGLE, DO THE SAME THING TURNING RIGHT
            // IF THE ANGLES ARE DIFFERENT THEN YOU KNOW YOU'RE NOT ON CENTER...
          }
          else if (currAttributes.endLeftPath || currAttributes.endRightPath ) { 
            startDistanceInMM = (int)(currDistanceTraveled*10);
            if (currAttributes.onLine == false) {
              // We're past the end of the intersection and there's no path in front of us
              waitForInstructions = true;
            }
            // Don't need to do anything other then send info to computer
          }
          else if (currAttributes.atEntrance ) {   // At entrance of the maze, nothing to do but proceed, computer will log this
            // Don't need to do anything
            startDistanceInMM = (int)(currDistanceTraveled*10);
          }
          else if (currAttributes.onLine) {  // If on a line nothing to do but keep going forward
            // Don't need to do anything
          }
          /* ------- We no longer need to do 'driftLogic' the adjustPositionOnLine should handle it, and because of memory limitations I commented this out
          else if (currAttributes.driftLeft || currAttributes.driftRight) {   // If have drifted off the line then adjust
            distanceToTravel -= movementsObj->getDistanceTraveledSoFar();     // Get remaining distance 
            movementsObj->stopMoving();                                       // Stop moving
            infraredObj->adjustForDrifting(currAttributes.driftLeft);         // Call routine to re-center us on the line
            delay(1000);
            setLastAttributes = false;
          }
          ------- */
          
          // Get the pose of the sensor
          poseOfSensor = infraredObj->getPoseOfCenterSensor();

          // If we're not moving and running in 'Goal state' then turn on check4Obstacle
          if (currentInstruction.instruction == GOAL_MODE && isMoving == false) 
            check4Obstacle = true;

          // If check4Obstacle then stop moving and take reading... reusing variable to identify if there is one 
          if (check4Obstacle == true) {  
            movementsObj->stopMoving(); 
            if (DEBUGINFRARED) {
              Serial.print("spaceInFrontOfGripper: ");
              Serial.println(ultrasonicObj->getFreeSpaceInFrontExcludingGripper(0));
            }
            float theNum = ultrasonicObj->getFreeSpaceInFrontExcludingGripper(0);
            Serial.print(F("Obst:"));
            Serial.println(theNum);
            if (theNum > INFRARED_MAX_GOAL_DISTANCE) check4Obstacle = false;
            //check4Obstacle = (ultrasonicObj->getFreeSpaceInFrontExcludingGripper(0) < INFRARED_MAX_GOAL_DISTANCE);
          }
            
          // If running in explor mode then send info to python program, we write out the state change, it's pose and the flag to say we found our goal
          if (currentInstruction.instruction == EXPLORE_MODE)
            infraredObj->showInfraredAttributes("StateChg",currAttributes, poseOfSensor, check4Obstacle);

          // If isMoving is off then we've reached destination, get the next destination; if resetStartPose then we've had the first
          //   state change... we need to reset our startPose to match the values from the mapping
          if (isMoving == false || resetStartPose == true) {
            waitForInstructions = true;            
          }
          
          // If the wait for instructions flag is on then wait :)
          if (waitForInstructions) {            
            if (DEBUGINFRARED) Serial.println("in waitForInstructions");
            
            // Stop so you can act on whatever instructions you're given
            movementsObj->stopMoving();          
            if (DEBUGINFRARED) Serial.println("stop");
            sparki.beep();
            delay(300);
            
            // Tell the python program that the prior instructions completed, we'll write out tag to say
            // INSSTOP,instructionCode,POSEValues...,Obst,ObstacleIndicator (obstacle only really valid on goas)
            // Strange, using locationObj->write* methods here is less memory than Serial.print... but it's move up above
            localizationObj->writeMsg2Serial("IR,INSSTOP,",false);
            localizationObj->writeChar2Serial(currentInstruction.instruction,false);
            localizationObj->writeChar2Serial(',',false);
            localizationObj->showPose(poseOfSensor,false);
            localizationObj->writeMsg2Serial(",Obst,",false);
            localizationObj->writeMsg2Serial((check4Obstacle ? "t" : "f"));
            //Serial.println(" ");
            //delay(10); 
            
            /*
            Serial.print("IR,INSSTOP,");
            Serial.print(currentInstruction.instruction);
            Serial.print(',');
            localizationObj->showPose(poseOfSensor,false);
            Serial.print(",Obst,");
            if (check4Obstacle) Serial.println("t");
            else Serial.println("f");
            */
            
            // Reset flag so that nex time thru the loop we give our state
            lastInstructionMode = ' ';

            // May want to change so it calls wait for instructions whenever it was in explore mode... otherwise
            // if had two explores in a row the first set of values would be lost
            
            if (queueOfInstructions.isEmpty() == true) {
              if (DEBUGINFRARED) Serial.println("c1");              // Show checkpoint 1
              
              // Get more instructions NOTE: This adds them to the QUEUE of instructions
              // It will return a boolean true if we got instructions and false otherwise... that's
              // only being used here for console output
              waitForInstructions = infraredObj->waitForInstructions(queueOfInstructions);
              if (waitForInstructions == false) { localizationObj->writeMsg2Serial("NoINS!"); }
              
              if (DEBUGINFRARED) {
                localizationObj->writeMsg2Serial("GotInstructions");
                //char buffer[instructions.length()+1];
                //instructions.toCharArray(buffer, instructions.length()+1);
                // localizationObj->writeMsg2Serial(*ptrInstructions);
  
                InfraredInstructions theIns;
                Serial.print("qOfIns,size:");
                Serial.println(queueOfInstructions.count());
                if (queueOfInstructions.isEmpty() == false) {                  
                  theIns = queueOfInstructions.peek();
                  Serial.print(" topItem,ins:"); Serial.print(theIns.instruction);
                  Serial.print(" x:"); Serial.print(theIns.pose.xPos);
                  Serial.print(" y:"); Serial.print(theIns.pose.yPos);
                  Serial.print(" <:"); Serial.println(theIns.pose.angle); 
                }
              }
            }  
            
            waitForInstructions = false;
            if (queueOfInstructions.isEmpty() == true) {
              if (DEBUGINFRARED) Serial.println("c2");  // Checkpoint 2
              // Tell python we're done
              localizationObj->writeMsg2Serial("IR,Done");
              distanceToTravel = 0.0;
              done = true;      
            }
            else {
              // We have instructions in the stack, do the top item
              currentInstruction = queueOfInstructions.pop();
              if (DEBUGINFRARED) {
                Serial.print("Ins:"); Serial.print(currentInstruction.instruction);
                Serial.print(" x:"); Serial.print(currentInstruction.pose.xPos);
                Serial.print(" y:"); Serial.print(currentInstruction.pose.yPos);
                Serial.print(" <:"); Serial.println(currentInstruction.pose.angle);
              }
              if (DEBUGINFRARED) { Serial.println("c3"); delay(10000); } // Checkpoint 3 and also delay for 10 seconds so you can read console :)
              switch (currentInstruction.instruction) {
                case EXPLORE_MODE:
                    //  if (localizationObj->closeEnuf(localizationObj->getCurrentAngle(),currentInstruction.pose.angle,5.0,true) == false) {
                  movementsObj->turnToAngle(currentInstruction.pose.angle);
                  distanceToTravel = INFRARED_MAX_DISTANCE_TO_TRAVEL;
                  //localizationObj->writeMsg2Serial("IR,PathStart");     // Send trigger to python that we're starting a new path
                  //localizationObj->showPose(infraredObj->getPoseOfCenterSensor());  // Tell it our current pose... this is the start position
                  break;
                case GOAL_MODE:                  
                  movementsObj->turnToAngle(currentInstruction.pose.angle);
                  // We should already be at goal... so set distanceToTravel to 0
                  distanceToTravel = 0.0;
                  //localizationObj->writeMsg2Serial("IR,GoalStart");
                  break;
                case DONE_MODE:
                  distanceToTravel = 0.0;
                  done = true;
                  break;
                case GOTO_MODE:
                  // Call routine to re-orient ourselves toward the goal, and set the distanceToTravel to be where we want to go
                  distanceToTravel = infraredObj->adjustAngleToPose(currentInstruction.pose);
                  //localizationObj->writeMsg2Serial("IR,GoToStart");
                  break;
                case SETPOSE_MODE:
                  distanceToTravel = 0.0;
                  resetStartPose   = false;
                  localizationObj->setCurrentXPosition(currentInstruction.pose.xPos);
                  localizationObj->setCurrentYPosition(currentInstruction.pose.yPos);
                default:
                  break;
              }
              // We changed position in switch statement, get the pose of the sensor
              poseOfSensor = infraredObj->getPoseOfCenterSensor();

              if ((distanceToTravel < 0.1) && (done == false)) {
                // make the distance 0, we don't need to move that minimal amt, that'll cause loop to come
                // back in and as for another instruction
                distanceToTravel = 0.0;
                //Serial.println("distanceToTravel < 0.1");
                //delay(1000);
                //waitForInstructions = true;
              }
            }
          }

/*          if (setLastAttributes) {
            // Move the currentAttributes into lastAttributes
            infraredObj->assignSourceAttributesToTarget(currAttributes, lastAttributes);
            //numStateChanges++;
            //done = (numStateChanges > 5);
          }
          else {
            setLastAttributes = true;
          } */
        }
      }
      movementsObj->stopMoving();
      
      localizationObj->writeMsg2Serial("IR,Done");
    #endif


    // ----------------------------------------------------------------------------------------
    // Preliminary navigation work - follow tape
    // ----------------------------------------------------------------------------------------
    #define TESTONLINE false
    #if TESTONLINE   

      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      InfraredAttributes baseAttr = infraredObj->getBaseAttributes();
      float distanceToTravel = 18;
      int distanceInterval2CheckLine = INFRARED_INTERVAL_2_CHECK_ON_LINE;
      
      sparki.beep(); 
      localizationObj->writeMsg2Serial("Move to tape");
      delay(5000);
     
      // the while is basically saying move forward a distance, the only time we check the ultrasonic distance is if we are in 'goal mode'
      while (movementsObj->moveForward(distanceToTravel,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) {
        if (movementsObj->getDistanceTraveledSoFar() > distanceInterval2CheckLine) {
          Serial.print("distTraveledSoFar:");
          Serial.println(movementsObj->getDistanceTraveledSoFar());
          
          distanceToTravel -= movementsObj->getDistanceTraveledSoFar();     // Get remaining distance for when we start moving
          Serial.print("distanceToTravel:");
          Serial.println(distanceToTravel);
          movementsObj->stopMoving();                                       // Stop moving
          distanceInterval2CheckLine = infraredObj->adjustPositionOnLine(distanceInterval2CheckLine);  // Call routine to re-center us on the line
          Serial.println(" ");
          Serial.print("distanceInterval2CheckLine: ");
          Serial.println(distanceInterval2CheckLine);
          continue;  // Go back to the start of the loop
        }
        delay(50);
      }
      
    #endif

    #define TESTCENTERONLINE false
    #if TESTCENTERONLINE   

      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      InfraredAttributes baseAttr = infraredObj->getBaseAttributes();
      sparki.beep(); 
      Serial.println("Move to line");
      delay(3000);
      for (int i = 0; i < 3; i++) {
        delay(1000);
        infraredObj->adjustToLineCenter();
        localizationObj->writeMsg2Serial("Done");
      }
      Serial.println("Done");
      Serial.flush();
    #endif

    #define TESTDISTANCE false
    #if TESTDISTANCE
      unsigned int start = millis();
      float theDist = 0.0;
      while(movementsObj->moveForward(200.0,5,false)) {
        if ((millis() - start) >= 10000) {
          Serial.println(millis() - start);
          theDist = movementsObj->getDistanceTraveledSoFar();
          Serial.println(theDist);
          movementsObj->stopMoving();
          break;
        }
      }

      while(movementsObj->moveBackward(theDist,100,false));
      
    #endif


    #define TESTADJUST2TAPE false
    #if TESTADJUST2TAPE   

      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      InfraredAttributes baseAttr = infraredObj->getBaseAttributes();
      sparki.beep(); 
      delay(3000);
      infraredObj->adjustToTape();
      Serial.println("Done");
      Serial.flush();
    #endif


    // ----------------------------------------------------------------------------------------
    // Test the logic for when we drift off a line... this should put sparki back in the center
    // ----------------------------------------------------------------------------------------
    #define TESTDRIFT false
    #if TESTDRIFT
      localizationObj->writeMsg2Serial("IA,TestingDrift");
      delay(3000);
      sparki.beep();      
      
      InfraredAttributes currAttributes;
      currAttributes.driftLeft  = false;   // Want to test left drift
      currAttributes.driftRight = !currAttributes.driftLeft;
      
      infraredObj->adjustForDrifting(currAttributes.driftLeft);  // if flag is off it'll adjust for right drift
      localizationObj->writeMsg2Serial("Done");
    #endif
    
    // ----------------------------------------------------------------------------------------
    // Show the infrared sensor readings.
    // ----------------------------------------------------------------------------------------
    #define SHOWREADINGS false
    #if SHOWREADINGS
      while (true) {
        infraredObj->showInfraredAttributes("MoveMe",infraredObj->getInfraredAttributesAtSensorPose());
        delay(500);
      }
    #endif

    
    #define TESTINIT false
    #if TESTINIT
      // Calculate world
      localizationObj->writeMsg2Serial("Have the sparki on a 'clean' area");
      delay(5000);
      sparki.beep();
    
      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      infraredObj->showInfraredAttributes("Base",infraredObj->getBaseAttributes());

      InfraredAttributes currAttributes, lastAttributes;
      lastAttributes = infraredObj->getBaseAttributes();
      bool done = false;
      while ( (movementsObj->moveForward(4,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) && (done == false) ) {
        currAttributes = infraredObj->getInfraredAttributesAtSensorPose();
        infraredObj->showInfraredAttributes("Curr",currAttributes);
        localizationObj->showLocation();
        delay(50);
      }      
      localizationObj->writeMsg2Serial("Done with TESTINIT");
    #endif


   
    #define TESTSTATECHANGE1 false
    #if TESTSTATECHANGE1
      localizationObj->writeMsg2Serial("Put sparki in front of door");
      delay(5000);
      sparki.beep();

      InfraredAttributes currAttributes, lastAttributes;
      
      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      infraredObj->showInfraredAttributes("Base",infraredObj->getBaseAttributes());
      
      //infraredObj->assignSourceAttributesToTarget(infraredObj->getBaseAttributes(), lastAttributes);
      lastAttributes = infraredObj->getBaseAttributes();
      //infraredObj->showInfraredAttributes("last",lastAttributes);

      int numStateChanges = 0;
      bool done = false;
      unsigned int baseTimer = millis();
      unsigned int currTimer;
      
      while ( (movementsObj->moveForward(13,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) && (done == false) ) {
        currAttributes = infraredObj->getInfraredAttributesAtSensorPose();
        currTimer = millis();
        Serial.print("time to measure:");
        Serial.println((float)(currTimer-baseTimer)/1000.0);
        baseTimer = currTimer;
        if (infraredObj->stateChanged(currAttributes, lastAttributes)) {
          localizationObj->writeMsg2Serial("state changed");
          movementsObj->stopMoving();
          
          delay(2000);
          // Move forward half the line width and check that the state is still different
          while (movementsObj->moveForward((infraredObj->getLineWidth()/2.0),ULTRASONIC_MIN_SAFE_DISTANCE,false));
          delay(2000);
          currAttributes = infraredObj->getInfraredAttributesAtSensorPose();
          if (infraredObj->stateChanged(currAttributes, lastAttributes)) {
            localizationObj->writeMsg2Serial("2nd check state changed - true");
            // State is still different... handle it
            infraredObj->assignSourceAttributesToTarget(currAttributes, lastAttributes);
            numStateChanges++;
            done = (numStateChanges > 2);
          }
          else localizationObj->writeMsg2Serial("2nd check state changed - false");
        }
        //currState = infraredObj->getStateForAttributes(currAttributes);
        infraredObj->showInfraredAttributes("Curr",currAttributes);
        localizationObj->showLocation();
        delay(50);
      }
      movementsObj->stopMoving();
      
      localizationObj->writeMsg2Serial("Done");
    #endif
 
  }


}
