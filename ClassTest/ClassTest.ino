#include <Sparki.h>  // include the sparki library
#include "car.h"

int count = 0;

void setup() {
  
}


void loop() {
  // put your main code here, to run repeatedly:
  if (count == 0) {    
    count++;
    if (true == true) {
      Car myCar(1);
      sparki.clearLCD(); // wipe the LCD clear
      //sparki.print("Address of Car: ");
      //sparki.println(&myCar);
      sparki.print("Sizeof(Car): ");
      sparki.print(sizeof(myCar));
      sparki.print(" doors");
      sparki.println(myCar.getDoors());
      
      //sparki.print(theAngle);
      //sparki.print(" is: ");
      //float test = modAngle(theAngle);
      //sparki.println(test);
      sparki.updateLCD(); // put the drawings on the screen
    }
  }
}
  

