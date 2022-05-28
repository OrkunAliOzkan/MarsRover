#include <Robojax_L298N_DC_motor.h>
#include <cmath>
#include <iostream>
#include <Arduino.h>
#include "WiFi.h"
#include <SPI.h>
#include "code_body.h"
// motor 1 settings
#define CHA 0
// this pin must be PWM enabled 
// pin if Arduino board is used
#define PWMA 17 // PWMA 
#define AIN1 14 // AIN1
#define AIN2 16 //AIN2
// motor 2 settings
#define BIN1 4 //BIN1
#define BIN2 15 //BIN2
// this pin must be PWM enabled 
// pin if Arduino board is used
#define PWMB 2 // PWMB 
#define CHB 1

const int CCW = 2; // do not change
const int CW  = 1; // do not change
double motor1_calibration = 1;
double motor2_calibration = 0.9;
double motor1_strength = 0;
double motor2_strength = 0;
#define motor1 1 // left
#define motor2 2 // right
//  Name of network
    #define WIFI_SSID       "Siva's iPhone"    
//  Password
    #define WIFI_PASSWORD  "sivashanth"   
//  State mashine indicator
  bool isConnected = false;
  String in;
  
char key = ' ';
/*Robojax_L298N_DC_motor robot(
                                AIN1, 
                                AIN2, 
                                PWMA, 
                                CHA,  
                                BIN1, 
                                BIN2, 
                                PWMB, 
                                CHB);
*/
// for two motors with debug information
Robojax_L298N_DC_motor robot(
                                AIN1, 
                                AIN2, 
                                PWMA, 
                                CHA, 
                                BIN1, 
                                BIN2, 
                                PWMB, 
                                CHB, 
                                true);
void setup() {
  Serial.begin(115200);
  robot.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Initializing");
}


void loop() {
//  If desired to connect 
//  and can, express as connected
    //Serial.println(WiFi.status());
  motor1_strength = 70 * motor1_calibration;
  motor2_strength = 70 * motor2_calibration;
  Serial.println(motor1_strength);
  Serial.println(round(motor1_strength));
  Serial.println(motor2_strength);

    if (WiFi.status() == WL_CONNECTED || 
        !isConnected) {
        Serial.println("Connected");
        //digitalWrite(LED, HIGH);
        isConnected = true;

        //  Body of code
        //code_body.test_code();
        //Serial.println("a");
        in = code_body.HTTPGET();
        //Serial.println("b");
/////////////////////////////////////////////////////
        if  ( (in == "w") || 
              (in == "a") || 
              (in == "s") || 
              (in == "d") ||
              (in == "W") || 
              (in == "A") || 
              (in == "S") || 
              (in == "D"))
        {
          key = in[0];
          Serial.flush();
          robot.rotate(motor1, motor1_strength, CW);
          robot.rotate(motor2, motor2_strength, CCW);
          delay(100);
          //while (Serial.available()) {
          //  key = Serial.read();
          //delay(10);
          //}
        }
        //  If not WASD
        else if(in == ".")
        {
          robot.rotate(motor1, 0, CW);
          robot.rotate(motor2, 0, CCW); 
        }
/////////////////////////////////////////////////////
        delay(200);
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
