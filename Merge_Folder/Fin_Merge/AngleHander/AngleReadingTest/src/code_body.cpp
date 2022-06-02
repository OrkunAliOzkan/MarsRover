#include "Arduino.h"
#include <SPI.h>
#include <vector>
#include <HTTPClient.h>
#include "QMC5883LCompass.h"
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

void fclass::readings(int counter_input, 
                                QMC5883LCompass *compass, 
                                float *angle, 
                                float *headingDegrees)
{
    if((counter_input % 2) == 0)
    {
    //  Compass readings
        compass->read();
        float x_comp = compass->getX();
        float y_comp = compass->getY();    
        delay(50);  
        *angle = code_body.computeAngle(x_comp, y_comp);
        *headingDegrees = ((*angle) * (180/M_PI));
        //Serial.println(*headingDegrees);
    }
    if((counter_input % 2) != 0)
    {
    //  TODO: Optical flow sensor readings
    }
}


fclass code_body = fclass();