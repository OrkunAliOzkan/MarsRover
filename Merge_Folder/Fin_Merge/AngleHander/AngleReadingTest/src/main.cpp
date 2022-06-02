#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BusIO_Register.h>
#include "QMC5883LCompass.h"
#include "code_body.h"
#include "PID.h"  //  https://github.com/cvra/pid
#include <Adafruit_BusIO_Register.h>
#include <Wire.h>
#include <cmath>
#include <vector>
///////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////
pid_ctrl_t pid;

float x = 0;
float y = 0;

float previous_angle = 0;
float requested_angle = 0;

float perpendicular_distance = 0;
float adjustment_angle = 0;

float orthogonal = 0;
std::vector<float> projection = {0, 0};
std::vector<float> actual = {0, 0};
std::vector<float> desired = {0, 0};


float acutal_angular_velocity = 0;
float previous_angle = 0;
///////////////////////////////////////////////////////////////////
QMC5883LCompass compass;
int x_comp, y_comp;
double angle;
float headingDegrees;
#define DECLINATIONANGLE 0.483 * /*(PI / 180)*/
int counter_input = 0;
///////////////////////////////////////////////////////////////////
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
  //  TODO: Get the desired angle from HTTP.GET in the loop
  requested_angle = 0;

  pid_init(&pid);

/* PD controller. */
pid_set_gains(&pid, 1, 1, 1);
}

void loop()
{
///////////////////////////////////////////////////////////////////
//  Input readings
if((counter_input % 2) == 0)
{
  x = (analogRead(XJOY))/4;
  y = (analogRead(YJOY))/4;

//if joystick's y-axis potentiometer output is low, go forward
  if(y < 400) 
  {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    speedA = map(y, 400, 0, 0, 255);
    speedB = map(y, 400, 0, 0, 255);
  }

//if joystick's x-axis potentiometer output is high, go backwards
  else if (y > 700) 
  {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    speedA = map(y, 700, 1023, 0, 255);
    speedB = map(y, 700, 1023, 0, 255);
  }

  else 
  {
    speedA = 0; speedB = 0;
  }

  if(x < 400)
  {
    int XMAP = map(x, 400, 0, 0, 255);
    speedA = speedA - XMAP;
    speedB = speedB + XMAP;
    if (speedA < 0) {speedA = 0;}
    if (speedB > 255) {speedB = 255;}
  }

  if(x > 700)
  {
    int XMAP = map(x, 700, 1023, 0, 255);
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

  if (speedA < 10) {
    speedA = 0;
  }
  if (speedB < 10) {
    speedB = 0;
  }
  //Serial.println(y);
  analogWrite(PWMA, speedA);
  analogWrite(PWMB, speedB);
}
///////////////////////////////////////////////////////////////////
if((counter_input % 2) != 0)
{
//  Compass readings
  compass.read();
  x_comp = compass.getX();
  y_comp = compass.getY();

  delay(50);

  angle = code_body.computeAngle(x_comp, y_comp);
  headingDegrees = angle * 180/M_PI;
  Serial.println(headingDegrees);
///////////////////////////////////////////////////////////////////
//  Compute perpendicular distance from the desired trjectory
  //  Compute projeciton
  orthogonal = code_body.vector_multiply(actual, desired);

  perpendicular_distance = sqrt((actual[0] * actual[0]) + 
                                (actual[1] * actual[1]));

  for(int i = 0; i < desired.size(); i++)
  {
    projection[i] = desired[i] - ((orthogonal/perpendicular_distance)*(actual[i]));
  }

  adjustment_angle = pid_process(&pid, perpendicular_distance);
///////////////////////////////////////////////////////////////////
  //  Utilising the adjustment angles
  //  TODO: place code from joystick input, and execute!
  x = 1023 * sin(adjustment_angle * (PI/180));
  y = 1023 * cos(adjustment_angle * (PI/180));

//if joystick's y-axis potentiometer output is low, go forward
  if(y < 400) 
  {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    speedA = map(y, 400, 0, 0, 255);
    speedB = map(y, 400, 0, 0, 255);
  }

//if joystick's x-axis potentiometer output is high, go backwards
  else if (y > 700) 
  {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    speedA = map(y, 700, 1023, 0, 255);
    speedB = map(y, 700, 1023, 0, 255);
  }

  else 
  {
    speedA = 0; speedB = 0;
  }

  if(x < 400)
  {
    int XMAP = map(x, 400, 0, 0, 255);
    speedA = speedA - XMAP;
    speedB = speedB + XMAP;
    if (speedA < 0) {speedA = 0;}
    if (speedB > 255) {speedB = 255;}
  }

  if(x > 700)
  {
    int XMAP = map(x, 700, 1023, 0, 255);
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
  //Serial.println(y);
  analogWrite(PWMA, speedA);
  analogWrite(PWMB, speedB);
///////////////////////////////////////////////////////////////////
/*
  We know angle and also motors PWM. One can
    //error = motor_position - setpoint;
    //motor_pwm = pid_process(&pid, error);
*/
//  Control readings
}
///////////////////////////////////////////////////////////////////
    counter_input++;
    delay(100);
}