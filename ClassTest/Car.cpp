#include <Sparki.h>
#include "car.h"

Car::Car(int anArg) {
  setDoors(anArg);
}

void Car::setDoors(int numDoors) {
  this->doors = numDoors;
}

int Car::getDoors() {
  return this->doors;
}
/* class Car {
  private: int doors;
           int wheels;
           byte color;
  public: 
    void setDoors(int numDoors) {
      doors = numDoors;
    }
    int getDoors() {
      return doors;
    }
};
*/
