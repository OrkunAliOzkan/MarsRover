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
/////////////////////////////////////////////////////////////////
//  Optical flow sensor parameters
#define PIN_SS                  5 //slave select/chip select (ESP32 pin number)
#define PIN_MISO                19 //output data from mouse sensor (ESP32 pin number)
#define PIN_MOSI                23 //input data to mouse sensor (ESP32 pin number)
#define PIN_SCK                 18 //synchronous clock (ESP32 pin number)
#define PIN_MOUSECAM_RESET      35 //reset (ESP32 pin number)
#define PIN_MOUSECAM_CS         5 //chip select (ESP32 pin number again)
#define ADNS3080_PIXEL_SUM      0x06
#define ADNS3080_PIXELS_X       30 //constant for x-axis pixel count 
#define ADNS3080_PIXELS_Y       30 //constant for y-axis pixel count 
#define ADNS3080_PIXEL_BURST    0x40 
#define ADNS3080_MOTION_BURST   0x50
#define ADNS3080_FRAME_CAPTURE  0x13 //read and write

int total_x_OFS = 0; //initialises actual total x-displacement from starting/reset position 
int total_y_OFS = 0; //initialises actual total y-displacement from starting/reset position 

int total_x1_OFS = 0; //total x displacement before applying scale factor
int total_y1_OFS = 0; //total y displacement before applying scale factor

int distance_x_OFS = 0;
int distance_y_OFS = 0;
/////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////
//  HTTP readings
  //  {MAG, ANGLE}
  std::vector<float> desired_polar      = {0, 0};
  std::vector<float> desired_cartesian  = {0, 0};
/////////////////////////////////////////////////////////////////
//  PID intance declaration
pid_ctrl_t pid;

//  Rovers displacement
float x = 0;
float y = 0;

//  parameters used in PID
float perpendicular_distance = 0;
float adjustment_angle = 0;
float magnitude = 0;
float projection_magnitude = 0;
float orthogonal = 0;
std::vector<float> projection = {0, 0};
std::vector<float> actual = {0, 0};
/////////////////////////////////////////////////////////////////
//  Compass readings and parameters
QMC5883LCompass compass;
int x_comp = 0, y_comp = 0;
float angle = 0;
float headingDegrees = 0;
#define DECLINATIONANGLE 0.483 /* * (PI / 180) */
int counter_input = 0;
/////////////////////////////////////////////////////////////////
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

/* PD controller declaration */
  pid_init(&pid);
  pid_set_gains(&pid, 0, 0, 0);
/////////////////////////////////////////////////////////////////
  pinMode(PIN_SS,OUTPUT); //sets the pin as an output pin
  pinMode(PIN_MISO,INPUT); //sets the pin as an input pin
  pinMode(PIN_MOSI,OUTPUT); //sets the pin as an output pin
  pinMode(PIN_SCK,OUTPUT); //sets the pin as an output pin

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32); //sets SPI clock to 1/32 of the ESP32's clock
  SPI.setDataMode(SPI_MODE3); //in SPI mode 3, data is sampled on the falling edge and shifted out on the rising edge
  SPI.setBitOrder(MSBFIRST);


  if(code_body.mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }
}

void loop()
{
/////////////////////////////////////////////////////////////////
/*
    TODO: Merge the input from HTTP request to get what is my
    desired displacement, from magnitude and angle.
*/
  //  magnitude and angle
  desired_polar[0] = 10;
  desired_polar[1] = 30;
  //  x & y
  desired_cartesian[0] = desired_polar[0]*sin(desired_polar[1]);
  desired_cartesian[1] = desired_polar[0]*cos(desired_polar[1]);
/////////////////////////////////////////////////////////////////
//  Input readings
  x = (analogRead(XJOY))/4 - 512;
  y = (analogRead(YJOY))/4 - 512;
/////////////////////////////////////////////////////////////////
//  Readings
code_body.readings(
                        counter_input, 
                        &compass, 
                        &angle, 
                        &headingDegrees,
                        &distance_x_OFS,
                        &distance_y_OFS,
                        &total_x1_OFS,
                        &total_y1_OFS,
                        &total_x_OFS,
                        &total_y_OFS
                        );
/*
x_OFS;
y_OFS;
a_OFS;
b_OFS;
*/

  /*
  compass.read();
  x_comp = compass.getX();
  y_comp = compass.getY();

  delay(50);

  angle = code_body.computeAngle(x_comp, y_comp);
  headingDegrees = angle * 180/M_PI;
  Serial.println(headingDegrees);
  */
/////////////////////////////////////////////////////////////////
//    Compute magnitude (avg to be more exact)
  magnitude = y / sin(angle);
  magnitude += x / cos(angle);
  magnitude /= 2;
/////////////////////////////////////////////////////////////////
//  Compute perpendicular distance from the desired trjectory
  //  Compute projeciton
  orthogonal = code_body.vector_multiply(
                                        actual, 
                                        desired_cartesian
                                        );

  perpendicular_distance = sqrt((actual[0] * actual[0]) + 
                                (actual[1] * actual[1]));

  for(int i = 0; i < desired_cartesian.size(); i++)
  {
    projection[i] = desired_cartesian[i] - 
              ((orthogonal/perpendicular_distance)*(actual[i]));
  }

  projection_magnitude = sqrt((projection[0] * projection[0]) + 
                              (projection[1] * projection[1]));

  adjustment_angle = pid_process(&pid, perpendicular_distance);
/////////////////////////////////////////////////////////////////
//  Utilising the adjustment angles
  x = magnitude*sin(
                  (headingDegrees + adjustment_angle) * (PI/180)
                  );
  y = magnitude*cos(
                  (headingDegrees + adjustment_angle) * (PI/180)
                  );
  /*
  Serial.println("-------------");
  Serial.println(x);
  Serial.println(y);
  Serial.println("-------------");
  */

//if joystick's y-axis potentiometer output is low, go forward
  if(y < -112) 
  {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    speedA = map(y, 400, 0, 0, 255);
    speedB = map(y, 400, 0, 0, 255);
  }

//if joystick's x-axis potentiometer output is high, go backwards
  else if (y > 188) 
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

  if(x < -112)
  {
    int XMAP = map(x, 400, 0, 0, 255);
    speedA = speedA - XMAP;
    speedB = speedB + XMAP;
    if (speedA < 0) {speedA = 0;}
    if (speedB > 255) {speedB = 255;}
  }

  if(x > 188)
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
/////////////////////////////////////////////////////////////////
  counter_input++;
  delay(100);
}