#include <Arduino.h>
#include <SPI.h>
#include "pid.h"
//////////////////////////////////////////////////////////////////
//  angleue defines the angle. 
double angle = 7;
double legacyAngle = angle;
double left_proportion = 0.5;
double right_proportion = 0.5;
PID pid = PID(0.1, 100, -100, 0.1, 0.01, 0.5);
//////////////////////////////////////////////////////////////////
void setup()
{
  Serial.println("hello");
}

void loop()
{
  //  The setpoint is the angle in mind.
  double inc = pid.calculate(0, angle);
  printf("-----------------------------\n");
  printf( "angle:% 7.3f\tinc:% 7.3f\nLeft:% 7.3f\tRight:% 7.3f\n", 
        angle, inc, 
        left_proportion, 
        right_proportion);
  legacyAngle = angle;
  angle += inc;
  if(inc < 0)
  {
    left_proportion *= abs(angle/legacyAngle);
  }
  else if(inc > 0)
  {
    right_proportion *= abs(angle/legacyAngle);
  }
  delay(1000);
}