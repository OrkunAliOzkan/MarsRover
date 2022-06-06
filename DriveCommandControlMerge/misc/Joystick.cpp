#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BusIO_Register.h>
#include "QMC5883LCompass.h"
#include <Adafruit_BusIO_Register.h>
#include <Wire.h> 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
#define XJOY 27
#define YJOY 26

int speedA = 0;
int speedB = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMC5883LCompass compass;
int x, y, z;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  compass.init();

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(XJOY, INPUT);
  pinMode(YJOY, INPUT);
  Serial.begin(9600);
}

void loop()
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  float x_joy = (analogRead(XJOY))/4;;
  float y_joy = (analogRead(YJOY))/4;;

  if(y_joy < 400) //if joystick's y-axis potentiometer output is low, go forwards (wired joystick y-axis is inverted!?!?) 
  {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    
    speedA = map(y_joy, 400, 0, 0, 255);
    speedB = map(y_joy, 400, 0, 0, 255);

  }

  else if (y_joy > 700) //if joystick's x-axis potentiometer output is high, go backwards
  {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    speedA = map(y_joy, 700, 1023, 0, 255);
    speedB = map(y_joy, 700, 1023, 0, 255);
    
  }

  else 
  {
    speedA = 0; speedB = 0;
  }

  if(x_joy < 400)
  {
    int XMAP = map(x_joy, 400, 0, 0, 255);
    speedA = speedA - XMAP;
    speedB = speedB + XMAP;
    if (speedA < 0) {speedA = 0;}
    if (speedB > 255) {speedB = 255;}
  }

  if(x_joy > 700)
  {
    int XMAP = map(x_joy, 700, 1023, 0, 255);
    speedA = speedA + XMAP;
    speedB = speedB - XMAP;
    if (speedA > 255) {speedA = 255;}
    if (speedB > 0) {speedB = 0;}
  }

//  if (x_joy < 40 && y_joy < 40)
//  {
//    speedA = 0;
//    speedB = 0; 
//  }

  if (speedA < 10) {
    speedA = 0;
  }
  if (speedB < 10) {
    speedB = 0;
  }
    Serial.println(y_joy);
    analogWrite(PWMA, speedA);
    analogWrite(PWMB, speedB);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Read compass values
    compass.read();

    x = compass.getX();
    y = compass.getY();
    z = compass.getZ();

    Serial.print("X: ");
    Serial.print(x);
    Serial.print("   Y: ");
    Serial.print(y);
    Serial.print("   Z: ");
    Serial.println(z);

    delay(300);
}