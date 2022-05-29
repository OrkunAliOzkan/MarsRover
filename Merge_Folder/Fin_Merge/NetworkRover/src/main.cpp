#include <Robojax_L298N_DC_motor.h>
#include <cmath>
#include <iostream>
#include <Arduino.h>
#include "WiFi.h"
#include <SPI.h>
#include "code_body.h"
#include "pid.h"
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
/////////////////////////////////////////////////////
const int CCW = 2; // do not change
const int CW  = 1; // do not change
double motor1_calibration = 1;
double motor2_calibration = 0.9;
double motor1_strength = 0;
double motor2_strength = 0;
#define motor1 1 // left
#define motor2 2 // right
//  Name of network
  #define WIFI_SSID       "DESKTOP-4UGQK2B 0362"    
//  Password
  #define WIFI_PASSWORD  "Bt133+98"   
//  State mashine indicator
  bool isConnected = false;
  String in;
  char key = ' ';
/////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Initializing");
// Network pin assignment ouput
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
//  If desired to connect 
//  and can, express as connected
    //Serial.println(WiFi.status());
      Serial.println("x");
  /*
    Serial.println(motor1_strength);
    Serial.println(round(motor1_strength));
    Serial.println(motor2_strength);
  */

    if ((WiFi.status() == WL_CONNECTED))
    {
      Serial.println("a");
     //|| !isConnected) 
        Serial.println("Connected");
        //digitalWrite(LED, HIGH);
        isConnected = true;

      //  Body of code
      Serial.println("b");
        in = code_body.HTTPGET();
      Serial.println("c");
        //Serial.println("b");
/////////////////////////////////////////////////////
// TODO: Make work with all input types
        if  ( (in == "w") || (in == "W") || 
              (in == "a") || (in == "A") || 
              (in == "s") || (in == "S") || 
              (in == "d") || (in == "D")
            )
        {
          Serial.flush();
      Serial.println("d");

          //  Case not supported. Could pretty up l8r
          if((in == "w") || (in == "W"))
          {
            code_body.Forward(1000,50);
          }
          else if((in == "d") || (in == "D"))
          {
            code_body.Backward(1000,50);
          }
          /*
          robot.rotate(motor1, motor1_strength, CW);
          robot.rotate(motor2, motor2_strength, CCW);
          */
          delay(100);
          //while (Serial.available()) 
          /*
          {
            key = Serial.read();
          delay(10);
          }
          */
      Serial.println("e");
        }

        //  If not WASD, do nothing
        else if(in == ".")
        {
          //robot.rotate(motor1, 0, CW);
          //robot.rotate(motor2, 0, CCW); 
        }
/////////////////////////////////////////////////////
        delay(2000);
    }

//  If not connected, connect 
//  and express as not connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
        //digitalWrite(LED, 
        //            !digitalRead(LED));
        delay(1000);
        isConnected = false;
    }
        //Serial.println("?");
}
