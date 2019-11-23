#include <Sparki.h> // include the robot library

#define INFRARED_LINE_THRESHOLD .10
#define INFRARED_SENSOR_FORWARD_OF_CENTER 4
#define INFRARED_DRIFT_ADJUSTMENT_DEGREES 3

int counter;
void setup()
{
  counter = 0;
  Serial.begin(9600);
  sparki.beep();
  delay(5000);  
  Serial.setTimeout(10000);  // Set se
}

// Little helper to return if reading is identifying tape or not
boolean isReadingOn(const int &reading, const int &baseAmt) {
  return ((baseAmt - reading) > (int)(baseAmt * INFRARED_LINE_THRESHOLD));
}

boolean onIntersection(const int &baseReading) {
  return (isReadingOn(sparki.edgeLeft(),baseReading) && isReadingOn(sparki.edgeRight(),baseReading));
}

int calcAngleFromEdgeToTape(const boolean &leftEdge, const int &baseReading, const int &millisFor90Degrees) {
  unsigned int startTime;
  unsigned int endTime;
  unsigned int theReading;
  int consecutiveReadingsOn = 0;

  startTime = millis();
  if (leftEdge) 
    sparki.moveRight();
  else
    sparki.moveLeft();
  while ((consecutiveReadingsOn < 3) && ((millis()-startTime) < millisFor90Degrees)) {
    if (leftEdge) 
      theReading = sparki.edgeLeft();
    else
      theReading = sparki.edgeRight();
    if (isReadingOn(theReading,baseReading)) 
      consecutiveReadingsOn++;
    else
      consecutiveReadingsOn = 0;
    delay(3);
  }
  endTime = millis();
  sparki.moveStop();
  unsigned int degrees2Return = (int)( ( (float)(endTime-startTime)/(float)(millisFor90Degrees) ) * 90.0 + 0.5);
  if (leftEdge) 
    sparki.moveLeft(degrees2Return);
  else
    sparki.moveRight(degrees2Return);
  return degrees2Return;
}

// ---------------------------------------------------------------------------------------------------------
float degreesToRadians(const int &degrees) {
  return (degrees * (PI / 180.0));
}

float yDistanceForAngle(const int &theAngle, const int &distanceMoved) {
  return distanceMoved * sin(degreesToRadians(theAngle));
}

 
void loop()
{
  if (counter == 0) {
    counter++;

    if (true == false) {      
      unsigned int time1 = millis();
      sparki.moveRight(90);
      unsigned int time2 = millis();
      Serial.print("millis to move right 90':");
      Serial.println(time2-time1);
      delay(1000);
    
      time1 = millis();
      sparki.moveLeft(90);
      time2 = millis();
      Serial.print("millis to move left 90':");
      Serial.println(time2-time1);
      delay(1000);

      unsigned int delta = time2 - time1;
      time1 = millis();
      sparki.moveRight();
      while ((millis()-time1) < delta);
      sparki.moveStop(); // stop all robot wheels

      // try turning in 10' increments
      unsigned int delta10 = delta/10;
      for (int i = 0; i < 10; i++) {
        time1 = millis();
        sparki.moveLeft();
        while ((millis()-time1) < delta10);
        sparki.moveStop();
        delay(500);      
      } 
    }

    if (true == true) {
      int millisFor90Degrees = 2439;

      int maxDistance = 14;
      int distance2Check = 1;
      int totalDistance = 0;
      int deltaAngle;
      while (totalDistance < maxDistance) {
        sparki.moveForward(distance2Check);
        totalDistance += distance2Check;
        if (onIntersection(970) == false) {
          int leftAngle = calcAngleFromEdgeToTape(true, 970, millisFor90Degrees);
          int rightAngle = calcAngleFromEdgeToTape(false, 970, millisFor90Degrees);
          deltaAngle = abs(leftAngle - rightAngle);
              
          Serial.print("left angle: ");
          Serial.println(leftAngle);
          Serial.print(" right angle: ");
          Serial.print(rightAngle);
          Serial.print(" deltaAngle: ");
          Serial.println(deltaAngle);
          if (deltaAngle > INFRARED_DRIFT_ADJUSTMENT_DEGREES) {
            deltaAngle = (deltaAngle + 1)/2 + 1;  // Move 1 degree more than 1/2 way
            if (leftAngle > rightAngle) {
              sparki.moveRight(deltaAngle);
            }
            else {
              sparki.moveLeft(deltaAngle);
            }
            distance2Check = 1;
          }
          else // No drift increase the distance to check
            distance2Check += distance2Check;
        }
        else // On an instersection go back to 1
          distance2Check = 1;
      }
/*
      float leftDistance = yDistanceForAngle(leftAngle, INFRARED_SENSOR_FORWARD_OF_CENTER); 
      float rightDistance = yDistanceForAngle(righttAngle, INFRARED_SENSOR_FORWARD_OF_CENTER); 
      if (leftDistance > rightDistance) {
        deltaFromCenter = leftDistance - rightDistance;
      }
      else {
        deltaFromCenter = rightDistance - leftDistance;
        
      }
      float deltaFromCenter = 
}
*/
      
    }

    
    
  }
}
