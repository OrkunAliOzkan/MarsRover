#include <cmath>
#include <iostream>
#include <Arduino.h>
#include "WiFi.h"
#include <SPI.h>
#include "code_body.h"  //  Client code + motion
#include "pid.h"        // PID controller
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
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
#define XJOY 27
#define YJOY 26

//  Less is more in this case
#define MAX_RANGE 480
#define MIN_RANGE 550

int speedA = 0;
int speedB = 0;
/////////////////////////////////////////////////////////////
//double inc;
//double angle = 7;
//double legacyAngle = angle;
//double left_proportion = 0.5;
//double right_proportion = 0.5;
//PID pid = PID(0.1, 100, -100, 0.1, 0.01, 0.5);
/////////////////////////////////////////////////////////////
// these pins may be different on different boards
//
//#define PIN_SS                                      5
//#define PIN_MISO                                    19
//#define PIN_MOSI                                    23
//#define PIN_SCK                                     18
//
//#define PIN_MOUSECAM_RESET                          35
//#define PIN_MOUSECAM_CS                             5
//
//#define ADNS3080_PIXELS_X                           30
//#define ADNS3080_PIXELS_Y                           30
//
//#define ADNS3080_PRODUCT_ID                         0x00
//#define ADNS3080_REVISION_ID                        0x01
//#define ADNS3080_MOTION                             0x02
//#define ADNS3080_DELTA_X                            0x03
//#define ADNS3080_DELTA_Y                            0x04
//#define ADNS3080_SQUAL                              0x05
//#define ADNS3080_PIXEL_SUM                          0x06
//#define ADNS3080_MAXIMUM_PIXEL                      0x07
//#define ADNS3080_CONFIGURATION_BITS                 0x0a
//#define ADNS3080_EXTENDED_CONFIG                    0x0b
//#define ADNS3080_DATA_OUT_LOWER                     0x0c
//#define ADNS3080_DATA_OUT_UPPER                     0x0d
//#define ADNS3080_SHUTTER_LOWER                      0x0e
//#define ADNS3080_SHUTTER_UPPER                      0x0f
//#define ADNS3080_FRAME_PERIOD_LOWER                 0x10
//#define ADNS3080_FRAME_PERIOD_UPPER                 0x11
//#define ADNS3080_MOTION_CLEAR                       0x12
//#define ADNS3080_FRAME_CAPTURE                      0x13
//#define ADNS3080_SROM_ENABLE                        0x14
//#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER       0x19
//#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER       0x1a
//#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER       0x1b
//#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER       0x1c
//#define ADNS3080_SHUTTER_MAX_BOUND_LOWER            0x1e
//#define ADNS3080_SHUTTER_MAX_BOUND_UPPER            0x1e
//#define ADNS3080_SROM_ID                            0x1f
//#define ADNS3080_OBSERVATION                        0x3d
//#define ADNS3080_INVERSE_PRODUCT_ID                 0x3f
//#define ADNS3080_PIXEL_BURST                        0x40
//#define ADNS3080_MOTION_BURST                       0x50
//#define ADNS3080_SROM_LOAD                          0x60
//
//#define ADNS3080_PRODUCT_ID_VAL                     0x17
//
//
//int total_x = 0;
//int total_y = 0;
//
//int total_x1 = 0;
//int total_y1 = 0;
//
//int x=0;
//int y=0;
//
//int a=0;
//int b=0;
//
//int distance_x=0;
//int distance_y=0;
//
//volatile byte movementflag=0;
//volatile int xydat[2];
//
//int tdistance = 0;
//
//struct MD
//{
//    byte motion;
//    char dx, dy;
//    byte squal;
//    word shutter;
//    byte max_pix;
//};
/////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);
//  Network initialization
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Initializing");
//  Drive pin assignment ouput
    pinMode(PWMA,       OUTPUT);
    pinMode(PWMB,       OUTPUT);
    pinMode(AIN1,       OUTPUT);
    pinMode(AIN2,       OUTPUT);
    pinMode(BIN1,       OUTPUT);
    pinMode(BIN2,       OUTPUT);
    pinMode(XJOY,       INPUT);
    pinMode(YJOY,       INPUT);
    //pinMode(STANDBY,  OUTPUT);    SIVA DO WE need this?
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
        float x = (analogRead(XJOY))/4;;
        float y = (analogRead(YJOY))/4;;
        // ANgle computaiton
        float angle = arctan(y/x);
    
        if(y < MAX_RANGE) //if joystick's y-axis potentiometer output is low, go forwards (wired joystick y-axis is inverted!?!?) 
        {
            digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
            digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);

            speedA = map(y, MAX_RANGE, 0, 0, 255);
            speedB = map(y, MAX_RANGE, 0, 0, 255);
        }
    
        else if (y > MIN_RANGE) //if joystick's x-axis potentiometer output is high, go backwards
        {
            digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
            digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
            speedA = map(y, MIN_RANGE, 1023, 0, 255);
            speedB = map(y, MIN_RANGE, 1023, 0, 255);
        }
    
        else 
        {
            speedA = 0; speedB = 0;
        }
    
        if(x < MAX_RANGE)
        {
            int XMAP = map(x, MAX_RANGE, 0, 0, 255);
            speedA = speedA - XMAP;
            speedB = speedB + XMAP;
            if (speedA < 0) {speedA = 0;}
            if (speedB > 255) {speedB = 255;}
        }

        if(x > MIN_RANGE)
        {
            int XMAP = map(x, MIN_RANGE, 1023, 0, 255);
            speedA = speedA + XMAP;
            speedB = speedB - XMAP;
            if (speedA > 255) {speedA = 255;}
            if (speedB > 0) {speedB = 0;}
        }
    
    //  if (x < 40 && y < 40)
    //  {
    //    speedA = 0;
    //    speedB = 0; 
    //  }
    
    
        if (speedA < 10) 
        {
            speedA = 0;
        }
        if (speedB < 10) 
        {
            speedB = 0;
        }
            Serial.println(y);
            analogWrite(PWMA, speedA);
            analogWrite(PWMB, speedB);
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