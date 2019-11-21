#include <Sparki.h>

#include "StackArray.h"
#include "SplitString.h"

struct Pose {
  float xPos;
  float yPos;
  int angle;
};

struct InfraredInstructions {
  char instruction;  // X-explore, G-goal, Q-quit/done M-moveto (goto)
  Pose pose;         // has x, y (floats) and angle (int)
};


int count;
void setup() {
  // put your setup code here, to run once:
  count = 0;
  Serial.begin(9600);
  delay(5000);
}

void showSomething(const StackArray<char> &stackOfChar) {
  Serial.print("In showSomething: ");
  Serial.print(stackOfChar.pop());
  Serial.println(stackOfChar.pop());
}


void loop() {
  if (count == 0) {
    count++;
    // put your main code here, to run repeatedly:

  
    #define TESTPARSE false
    #if TESTPARSE
      String fluffy = "XPLORE,4.52,5.21,34,GOTO,9.2,8.7,4,GOTO,103.2,421.7,450";
  
      const char*  str_data       = fluffy.c_str();//"XPLORE,4.52,5.21,34,GOTO,9.2,8.7,4";
      size_t  num_parameters      = 0;
      size_t  len_parameter       = 0;
  
      param_count_and_length(str_data, num_parameters, len_parameter);
  
      const size_t PARAMS_MAX    = num_parameters;
      const size_t LENGTH_MAX    = len_parameter;
      char szParams[PARAMS_MAX][LENGTH_MAX + 1];
  
      char* pnext = (char*)extract_token(&szParams[0][0], str_data);
      Serial.print("szParams[0]: ");
      Serial.println(szParams[0]);
  
      for ( size_t i = 1; i < num_parameters; i++ )
      {
          pnext = (char*)extract_token(&szParams[i][0], pnext);
          Serial.print("szParams[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(szParams[i]);
      }
    #endif

    #define TESTSTACK false
    #if TESTSTACK
      // declare a string message.
      const String msg = "Happy Hacking!";

      // create a stack of characters.
      StackArray <char> stack;
      
      // set the printer of the stack.
      stack.setPrinter (Serial);
      
      // print the message before reversing.
      Serial.println ("Normal String: " + msg);

      // push all the message's characters to the stack.
      for (int i = 0; i < msg.length (); i++)
        stack.push (msg.charAt (i));

      showSomething(stack);
      
      // print the message after reversing.
      Serial.print ("Reversed String: ");

      // pop all the message's characters from the stack.
      while (!stack.isEmpty ())
        Serial.print (stack.pop ());

      // print end of line character.
      Serial.println ();

    #endif

    #define TESTSTACK2 false
    #if TESTSTACK2
      const size_t PARAMSMAX = 8;
      const size_t LENGTHMAX = 10;
      char sszParams[PARAMSMAX][LENGTHMAX + 1];
      String dummy = "12.34";
      sszParams[0][0] = "M\0";
      sszParams[1][0] = dummy.c_str();
      sszParams[2][0] = "5.6";
      sszParams[3][0] = "45";
      sszParams[4][0] = "X";
      sszParams[5][0] = "87.65";
      sszParams[6][0] = "7.86";
      sszParams[7][0] = "65";
      StackArray<InfraredInstructions> stackOfInstructions;
      InfraredInstructions theInstructions;
      int i = 0;
      while (i < 8) {
        switch( i % 4) {
          case 0:
            Serial.println(sszParams[i]);
            theInstructions.instruction = sszParams[i][0];
            Serial.print("ins: " );
            Serial.println(theInstructions.instruction);
            break;
          case 1: // x
            Serial.println(sszParams[i]);
            theInstructions.pose.xPos = atof(sszParams[i]);
            break;
          case 2: // y
            theInstructions.pose.yPos = atof(sszParams[i]);
            break;
          case 3: // angle
            theInstructions.pose.angle = atoi(sszParams[i]);
            stackOfInstructions.push(theInstructions);
            // theInstructions = new InfraredInstructions;
            break;
        }
        i++;
      }
    
      // pop all the message's characters from the stack.
      InfraredInstructions theIns;
      while (!stackOfInstructions.isEmpty ())
        theIns = stackOfInstructions.pop();
        Serial.print(theIns.instruction);
        Serial.print(" ");
        Serial.print(theIns.pose.xPos);
        Serial.print(" ");
        Serial.print(theIns.pose.yPos);
        Serial.print(" ");
        Serial.println(theIns.pose.angle);

      // print end of line character.
      Serial.println ();

    #endif

    #define TEST3 true
    #ifdef TEST3
      String fluffy = "X,4.52,5.21,34,M,9.2,8.7,4";
  
      const char*  str_data       = fluffy.c_str();//"XPLORE,4.52,5.21,34,GOTO,9.2,8.7,4";
      size_t  num_parameters      = 0;
      size_t  len_parameter       = 0;
  
      param_count_and_length(str_data, num_parameters, len_parameter);
  
      const size_t PARAMS_MAX    = num_parameters;
      const size_t LENGTH_MAX    = len_parameter;
      char szParams[PARAMS_MAX][LENGTH_MAX + 1];
  
      char* pnext = (char*)extract_token(&szParams[0][0], str_data);
      Serial.print("szParams[0]: ");
      Serial.println(szParams[0]);
  
      for ( size_t i = 1; i < num_parameters; i++ )
      {
          pnext = (char*)extract_token(&szParams[i][0], pnext);
          Serial.print("szParams[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(szParams[i]);
      }
      
      StackArray<InfraredInstructions> stackOfInstructions;
      InfraredInstructions theInstructions;
      int i = 0;
      while (i < 8) {
        switch( i % 4) {
          case 0:
            Serial.println(szParams[i]);
            theInstructions.instruction = szParams[i][0];
            Serial.print("ins: " );
            Serial.println(theInstructions.instruction);
            break;
          case 1: // x
            Serial.println(szParams[i]);
            theInstructions.pose.xPos = atof(szParams[i]);
            break;
          case 2: // y
            theInstructions.pose.yPos = atof(szParams[i]);
            break;
          case 3: // angle
            theInstructions.pose.angle = atoi(szParams[i]);
            stackOfInstructions.push(theInstructions);
            Serial.println("Pushed onto stack");
            // theInstructions = new InfraredInstructions;
            break;
        }
        i++;
      }
      Serial.println("Writing out stack");
      Serial.print("Stack size: ");
      Serial.println(stackOfInstructions.count());
      // pop all the message's characters from the stack.
      InfraredInstructions theIns;
      while (!stackOfInstructions.isEmpty ()) {
        theIns = stackOfInstructions.pop();
        Serial.print(theIns.instruction);
        Serial.print(" ");
        Serial.print(theIns.pose.xPos);
        Serial.print(" ");
        Serial.print(theIns.pose.yPos);
        Serial.print(" ");
        Serial.println(theIns.pose.angle);
      }
      // print end of line character.
      Serial.println ();

    #endif









    
  }
}
