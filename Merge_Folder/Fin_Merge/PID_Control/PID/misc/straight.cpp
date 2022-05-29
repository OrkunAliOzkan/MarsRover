#include <Arduino.h>
#include <SPI.h>
#include "pid.h"
////////////////////////////////////////////////////////////////
/*
  Value defines the angle. If this is
  negative, 
*/
double val = 7;
double legacyVal = val;
double left_proportion = 0.5;
double right_proportion = 0.5;
PID pid = PID(0.1, 100, -100, 0.1, 0.01, 0.5);
////////////////////////////////////////////////////////////////
void setup()
{
  Serial.println("hello");
}

void loop()
{
  double inc = pid.calculate(0, val);
  printf("-----------------------------\n");
  printf( "val:% 7.3f\tinc:% 7.3f\nLeft:% 7.3f\tRight:% 7.3f\n", 
        val, inc, 
        left_proportion, 
        right_proportion);
  legacyVal = val;
  val += inc;
  if(inc < 0)
  {
    left_proportion *= abs(val/legacyVal);
  }
  else if(inc > 0)
  {
    right_proportion *= abs(val/legacyVal);
  }
  delay(1000);
}