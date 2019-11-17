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
      
      localizationObj->writeMsg2Serial("IR,PathStart");
      float distanceToTravel = 17.0;  // Just some long distance for the first time  
      while ( (movementsObj->moveForward(distanceToTravel,ULTRASONIC_MIN_SAFE_DISTANCE,false) == true) && (done == false) ) {
        
        currAttributes = infraredObj->getInfraredAttributesAtCurrentPose();

        // This is just for debugging
        currTimer = millis();
        if (DEBUGINFRARED) {
          Serial.print("time to measure:");
          Serial.println((float)(currTimer-baseTimer)/1000.0);
        }
        baseTimer = currTimer;

        // We continue moving till the state changes
        if (infraredObj->stateChanged(currAttributes, lastAttributes)) {
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
            delay(2000);
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
      
          // If the wait for instructions flag is on then wait :)
          while (waitForInstructions) {
            // Stop moving and wait for instructions from computer on what to do
            // NOTE: we are off the tape here... was going to backup till I see tape but the distance is probably
            //       so small it's not worth it... may revisit after testing
            localizationObj->writeMsg2Serial("IR,PathEnd");
            movementsObj->stopMoving();            
            infraredObj->waitForInstructions();
            // CHANGE DOWN THE ROAD SO THAT IT PROCESSES THE INSTRUCTIONS
            done = true;
          }

          if (setLastAttributes) {
            // Move the currentAttributes into lastAttributes
            infraredObj->assignSourceAttributesToTarget(currAttributes, lastAttributes);
            numStateChanges++;
            done = (numStateChanges > 5);
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
