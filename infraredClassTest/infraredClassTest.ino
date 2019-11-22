#include "infraredClass.h"

byte counter = 0;
     
void setup() {
  // put your setup code here, to run once:
  #if USE_LCD 
    sparki.clearLCD();
    delay(500);
  #else
    Serial.begin(SERIAL_SPEED);
    sparki.beep();
    delay(5);
    sparki.beep();
    delay(DELAY_AFTER_SERIAL_STARTUP);  
    Serial.setTimeout(10000);  // Set serial timeout to 10 seconds
   
    // Clear anything on the serial port
    if (Serial.available() > 0)  {
      while (Serial.available() > 0) {
        char theChar = (char)Serial.read(); 
        delay(DELAY_FOR_SERIAL_COMM);
      }
    }
   
    
    sparki.beep();
  #endif
  counter = 0;
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
        currAttributes = infraredObj->getInfraredAttributesAtCurrentPose();
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
      
      infraredObj->assignSourceAttributesToTarget(infraredObj->getBaseAttributes(), lastAttributes);
      infraredObj->showInfraredAttributes("last",lastAttributes);

      int numStateChanges = 0;
      bool done = false;
      unsigned int baseTimer = millis();
      unsigned int currTimer;
      while ( (movementsObj->moveForward(13,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) && (done == false) ) {
        currAttributes = infraredObj->getInfraredAttributesAtCurrentPose();
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
          currAttributes = infraredObj->getInfraredAttributesAtCurrentPose();
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


    // ----------------------------------------------------------------------------------------
    // Preliminary navigation work - follow tape
    // ----------------------------------------------------------------------------------------
    #define TESTSTATECHANGE2 true
    #if TESTSTATECHANGE2
      QueueArray<InfraredInstructions> queueOfInstructions;
      InfraredInstructions currentInstruction;
    
      localizationObj->writeMsg2Serial("IR,FollowTape");
      delay(3000);
      sparki.beep();

      Pose poseOfSensor = infraredObj->getPoseOfCenterSensor();
      
      InfraredAttributes currAttributes, lastAttributes;
      
      infraredObj->setInfraredBaseReadings();  // Just updates the base structure
      infraredObj->showInfraredAttributes("Base",infraredObj->getBaseAttributes(),poseOfSensor,false);
      
      infraredObj->assignSourceAttributesToTarget(infraredObj->getBaseAttributes(), lastAttributes);
      infraredObj->showInfraredAttributes("last",lastAttributes,poseOfSensor,false);

      int numStateChanges = 0;
      bool done = false;
      unsigned int baseTimer = millis();
      unsigned int currTimer;

      bool waitForInstructions = false;  
      bool setLastAttributes = true;
      bool check4Obstacle = false;
      bool isMoving = false;
      
      localizationObj->writeMsg2Serial("IR,PathStart");
      float distanceToTravel = INFRARED_MAX_DISTANCE_TO_TRAVEL;  // Just some long distance for the first time  
      currentInstruction.instruction = EXPLORE_MODE;
      // the while is basically saying move forward a distance, the only time we check the ultrasonic distance is if we are in 'goal mode'
      while ( (movementsObj->moveForward(distanceToTravel,ULTRASONIC_MIN_SAFE_DISTANCE,(currentInstruction.instruction==GOAL_MODE)) == true) || (done == false) ) {
        
        currAttributes = infraredObj->getInfraredAttributesAtCurrentPose();
        isMoving       = movementsObj->isMoving();
        if (isMoving == false) Serial.println("isMoving is false");
        
        // This is just for debugging
        currTimer = millis();
        if (DEBUGINFRARED) {
          Serial.print("time to measure:");
          Serial.println((float)(currTimer-baseTimer)/1000.0);
        }
        baseTimer = currTimer;

        // We continue moving till the state changes
        if (infraredObj->stateChanged(currAttributes, lastAttributes) || (isMoving==false)) {
          if (DEBUGINFRARED) localizationObj->writeMsg2Serial("State Change");

          waitForInstructions = false;  // We want the computer to tell us where to go next
          check4Obstacle      = false;  // We only check this when at an exit
          
          // In this block we check the STATEs
          if (currAttributes.atExit) {  // We found an exit, stop moving and wait for instructions on where to go
            waitForInstructions = true;
            check4Obstacle = true;
          }
          else if (currAttributes.startLeftPath || currAttributes.startRightPath) {  // Start of path that's to left or right
            // *********** CHANGE SO THAT IT ENSURES IT'S AT INTERSECTION AND NOT DRIFTED INTO LINE.... THINKING
            // YOU COULD BACKUP A LITTLE, TURN LEFT TILL U SEE LINE KEEP TRACK OF ANGLE, DO THE SAME THING TURNING RIGHT
            // IF THE ANGLES ARE DIFFERENT THEN YOU KNOW YOU'RE NOT ON CENTER...
          }
          else if (currAttributes.endLeftPath || currAttributes.endRightPath ) { 
            if (currAttributes.onLine == false) {
              // We're past the end of the intersection and there's no path in front of us
              waitForInstructions = true;
            }
            // Don't need to do anything other then send info to computer
          }
          else if (currAttributes.atEntrance ) {   // At entrance of the maze, nothing to do but proceed, computer will log this
            // Don't need to do anything
          }
          else if (currAttributes.onLine) {  // If on a line nothing to do but keep going forward
            // Don't need to do anything
          }
          else if (currAttributes.driftLeft || currAttributes.driftRight) {   // If have drifted off the line then adjust
            distanceToTravel -= movementsObj->getDistanceTraveledSoFar();     // Get remaining distance 
            movementsObj->stopMoving();                                       // Stop moving
            infraredObj->adjustForDrifting(currAttributes.driftLeft);         // Call routine to re-center us on the line
            delay(1000);
            setLastAttributes = false;
          }

          // Get the pose of the sensor
          poseOfSensor = infraredObj->getPoseOfCenterSensor();
          if (check4Obstacle == true) {  // We're at an exit check to see if there's an obstacle in front of us
            movementsObj->stopMoving(); 
            check4Obstacle = (ultrasonicObj->getFreeSpaceInFrontExcludingGripper(0) < INFRARED_MAX_GOAL_DISTANCE);
          }
            
          // Write out the state change, it's pose and the flag to say we found our goal
          infraredObj->showInfraredAttributes("StateChg",currAttributes, poseOfSensor, check4Obstacle);

          // If isMoving is off then we've reached destination, get the next destination
          if (isMoving == false) {
            waitForInstructions = true;            
          }
          
          // If the wait for instructions flag is on then wait :)
          while (waitForInstructions) {
            Serial.println("in waitForInstructions");
            // Stop so you can act on whatever instructions you're given
            movementsObj->stopMoving();          

            // May want to change so it calls wait for instructions whenever it was in explore mode... otherwise
            // if had two explores in a row the first set of values would be lost
            
            if (queueOfInstructions.isEmpty() == true) {
              // Get more instructions
              // NOTE: we are off the tape here... was going to backup till I see tape but the distance is probably
              //       so small it's not worth it... may revisit after testing
              localizationObj->writeMsg2Serial("IR,PathEnd");
              String instructions = infraredObj->waitForInstructions(queueOfInstructions);
              if (DEBUGINFRARED) {
                localizationObj->writeMsg2Serial("GotInstructions");
                char buffer[instructions.length()+1];
                instructions.toCharArray(buffer, instructions.length()+1);
                localizationObj->writeMsg2Serial(buffer);
  
                InfraredInstructions theIns;
                Serial.print("queueOfInstructions, size: ");
                Serial.print(queueOfInstructions.count());
                if (queueOfInstructions.isEmpty() == false) {                  
                  theIns = queueOfInstructions.peek();
                  Serial.print(" top item, ins: ");
                  Serial.print(theIns.instruction);
                  Serial.print(" x: ");
                  Serial.print(theIns.pose.xPos);
                  Serial.print(" y: ");
                  Serial.print(theIns.pose.yPos);
                  Serial.print(" <: ");
                  Serial.println(theIns.pose.angle); 
                }
              }
            }  
            
            waitForInstructions = false;
            if (queueOfInstructions.isEmpty() == true) {
              // Tell python we're done
              localizationObj->writeMsg2Serial("IR,Done");
              done = true;      
            }
            else {
              // We have instructions in the stack, do the top item
              currentInstruction = queueOfInstructions.pop();
              Serial.print("Ins:");
              Serial.print(currentInstruction.instruction);
              Serial.print(" x:");
              Serial.print(currentInstruction.pose.xPos);
              Serial.print(" y:");
              Serial.print(currentInstruction.pose.yPos);
              Serial.print(" <:");
              Serial.println(currentInstruction.pose.angle);
              switch (currentInstruction.instruction) {
                case EXPLORE_MODE:
                    //  if (localizationObj->closeEnuf(localizationObj->getCurrentAngle(),currentInstruction.pose.angle,5.0,true) == false) {
                  movementsObj->turnToAngle(currentInstruction.pose.angle);
                  distanceToTravel = INFRARED_MAX_DISTANCE_TO_TRAVEL;
                  localizationObj->writeMsg2Serial("IR,PathStart");
                  break;
                case GOAL_MODE:
                  // The goal_mode is the pose of the sensor, we need to go to pose of center of bot so we remove INFRARED_SENSOR_FORWARD_OF_CENTER
                  distanceToTravel = infraredObj->adjustAngleToPose(currentInstruction.pose) - INFRARED_SENSOR_FORWARD_OF_CENTER;
                  localizationObj->writeMsg2Serial("IR,GoalStart");
                  break;
                case DONE_MODE:
                  distanceToTravel = INFRARED_MAX_DISTANCE_TO_TRAVEL;
                  done = true;
                  break;
                case GOTO_MODE:
                  // Call routine to re-orient ourselves toward the goal, and set the distanceToTravel to be where we want to go
                  distanceToTravel = infraredObj->adjustAngleToPose(currentInstruction.pose);
                  localizationObj->writeMsg2Serial("IR,GoToStart");
                  break;
                default:
                  break;
              }
              if ((distanceToTravel < 0.1) && (done == false)) {
                Serial.println("distanceToTravel < 0.1");
                delay(1000);
                waitForInstructions = true;
              }
            }
          }

          if (setLastAttributes) {
            // Move the currentAttributes into lastAttributes
            infraredObj->assignSourceAttributesToTarget(currAttributes, lastAttributes);
            numStateChanges++;
            //done = (numStateChanges > 5);
          }
          else {
            setLastAttributes = true;
          }
        }
      }
      movementsObj->stopMoving();
      
      localizationObj->writeMsg2Serial("IR,Done");
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
        infraredObj->showInfraredAttributes("MoveMe",infraredObj->getInfraredAttributesAtCurrentPose());
        delay(500);
      }
    #endif

  }




  
}
