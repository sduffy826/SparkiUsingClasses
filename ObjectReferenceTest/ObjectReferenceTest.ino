#include <Sparki.h>

class Point{
  private:
    int x, y;

  public: 
  Point() {
    this->x = 0;
    this->y = 0;
  }
  Point(const int &x1, const int &y1) {
    x = x1;
    y = y1;
  }
  void setPoint(const int &x1, const int &y1) {
    x = x1;
    y = y1;
    showPoint();  // Just to test forward call
  }
  int getPointX() { return x; }
  int getPointY() { return y; }
  void incPointX() { x++; }
  void incPointY() { y++; }
  void showPoint() {
    Serial.println("Point below");
    Serial.print("x,");
    Serial.print(x);
    Serial.print(",y,");
    Serial.println(y);
    delay(500);
  }
};

class Circle {
  private: Point *center;
           int radius;
  public:
  Circle() {
    //center.setPoint(0,0);
    radius = 0;
  }
  Circle(const Point &myCenter, const int &myRadius) {
    center = &myCenter;
    radius = myRadius;
  }
  void showCircle() {
    
    Serial.println("Circle below:");
    center->showPoint();
    Serial.print("r,");
    Serial.println(radius);
    delay(500);
  }
};



int counter = 0;
void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
 sparki.beep();
 delay(5000);  
 sparki.beep();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (counter == 0) {
    counter++;
    
    Point myPoint(2,2);
    Serial.println("pt.showPoint()");
    myPoint.showPoint();
    Serial.println(" ");
    
    Circle myCircle(myPoint, 5);
    Serial.println("circle showCircle()");
    myCircle.showCircle();
    Serial.println(" ");

    Serial.println(" ");
    Serial.println("After changing point ");
    myPoint.setPoint(5,5);
    Serial.println("pt.showPoint()");
    myPoint.showPoint();
    Serial.println(" ");
    
    Serial.println("circle showCircle()");
    myCircle.showCircle();

    callLater();
    
  }
}

void callLater() {
  Serial.println("foo");
}

