#include "Arduino.h"
#include <SPI.h>
#include <vector>
#include <HTTPClient.h>
#include "code_body.h"

#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
#define XJOY 27
#define YJOY 26

#define DECLINATIONANGLE 0.483 /* * (PI / 180)   FIXME: Isn't this in rad?*/

//int speedA = 0;
//int speedB = 0;

fclass::fclass()
{}
double fclass::computeAngle(int x, int y)
{
    int angle = atan2(y, x);
    angle += DECLINATIONANGLE;
    
    if(angle < 0) 
        angle += 2*PI; 
    if(angle > 2*PI)
        angle -= 2*PI;

    return angle;
}

float fclass::vector_multiply(std::vector<float> x, std::vector<float> y)
{
    //  It is known that x is actually a transpose
    float soln;
    for(int i = 0; i < x.size(); i++)
    {
        soln += (x[i]*y[i]);
    }

    return soln;
}


fclass code_body = fclass();