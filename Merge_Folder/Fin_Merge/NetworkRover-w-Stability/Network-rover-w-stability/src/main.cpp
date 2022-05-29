#include <cmath>
#include <iostream>
#include <Arduino.h>
#include "WiFi.h"
#include <SPI.h>
#include "code_body.h"  //  Client code + motion
#include "pid.h"        // PID controller
/////////////////////////////////////////////////////////////
// motor 1 settings
#define CHA     0
#define CHB     1
#define PWMA    17
#define PWMB    2
#define AIN1    14
#define AIN2    16
#define BIN1    4
#define BIN2    15
#define STANDBY 32
const int CCW = 2; // do not change
const int CW  = 1; // do not change
double motor1_calibration = 1;
double motor2_calibration = 0.9;
double motor1_strength = 0;
double motor2_strength = 0;
#define motor1 1 // left
#define motor2 2 // right
/////////////////////////////////////////////////////////////
//  Name of network
    #define WIFI_SSID       "DESKTOP-4UGQK2B 0362"
//  Password
    #define WIFI_PASSWORD   "Bt133+98"
//  State mashine indicator
    bool isConnected = false;
    String in;
    char key = ' ';
/////////////////////////////////////////////////////////////
double inc;
double angle = 7;
double legacyAngle = angle;
double left_proportion = 0.5;
double right_proportion = 0.5;
PID pid = PID(0.1, 100, -100, 0.1, 0.01, 0.5);
/////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);
//  Network initialization
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Initializing");
//  Drive pin assignment ouput
    pinMode(PWMA,     OUTPUT);
    pinMode(PWMB,     OUTPUT);
    pinMode(AIN1,     OUTPUT);
    pinMode(AIN2,     OUTPUT);
    pinMode(BIN1,     OUTPUT);
    pinMode(BIN2,     OUTPUT);
    pinMode(STANDBY,  OUTPUT);
}

void loop() 
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        Serial.println("Connected");
        isConnected = true;
    //  Body of code
        in = code_body.HTTPGET();
        /////////////////////////////////////////////////////
        if  (
            (in == "w") || (in == "W") || 
            (in == "a") || (in == "A") || 
            (in == "s") || (in == "S") || 
            (in == "d") || (in == "D")
            )
        {
        Serial.flush();
          //  Case not supported. Could pretty up l8r
            if
            (
                ((in == "w") || (in == "W")) ||
                ((in == "s") || (in == "S"))
            )
            {
                /////////////////////////////////////////////
                inc = pid.calculate(0, angle);
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
                /////////////////////////////////////////////
                if      ((in == "w") || (in == "W"))
                {
                    code_body.Forward
                    (
                        1000, 
                        100*left_proportion, 
                        100*right_proportion
                    );
                }
                else if ((in == "w") || (in == "W"))
                {
                    code_body.Backward
                    (
                        1000, 
                        100*left_proportion, 
                        100*right_proportion
                    );
                }
            }
            delay(100);
        }
    //  If not WASD, do nothing
        else if(in == ".")
        {
            /*TODO: What to do here?*/
        }
        /////////////////////////////////////////////////////
        delay(2000);
    }
//  If not connected, connect and express as not connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
        delay(1000);
        isConnected = false;
    }
}