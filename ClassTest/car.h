#include <Sparki.h>
class Car {
  private: int doors;
           int wheels;
           byte color;
  public: 
    Car(int doors);
    void setDoors(int numDoors);
    int getDoors();
};
