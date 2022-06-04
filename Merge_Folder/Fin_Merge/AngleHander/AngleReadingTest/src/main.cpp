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
#include "WiFi.h"
/////////////////////////////////////////////////////////////////
//  Optical flow sensor parameters
//slave select/chip select (ESP32 pin number)
  #define PIN_SS                  5 
//output data from mouse sensor (ESP32 pin number)
  #define PIN_MISO                19 
//input data to mouse sensor (ESP32 pin number)
  #define PIN_MOSI                23 
//synchronous clock (ESP32 pin number)
  #define PIN_SCK                 18 
//reset (ESP32 pin number)
  #define PIN_MOUSECAM_RESET      32 ////changed from 35! 
//chip select (ESP32 pin number again)
  #define PIN_MOUSECAM_CS         5 
#define ADNS3080_PIXEL_SUM      0x06
//constant for x-axis pixel count 
  #define ADNS3080_PIXELS_X       30 
//constant for y-axis pixel count 
  #define ADNS3080_PIXELS_Y       30 
//  Quick access registers that store pixel data
  #define ADNS3080_PIXEL_BURST    0x40 
//  Quick access registers that store motion data
  #define ADNS3080_MOTION_BURST   0x50
//read and write
  #define ADNS3080_FRAME_CAPTURE  0x13 

//initialises actual total x-displacement from starting position 
int total_x_OFS = 0; 
//initialises actual total y-displacement from starting position 
int total_y_OFS = 0; 

//total x displacement before applying scale factor
int total_x1_OFS = 0; 
 //total y displacement before applying scale factor
int total_y1_OFS = 0;

int distance_x_OFS = 0;
int distance_y_OFS = 0;
/////////////////////////////////////////////////////////////////
//  Name of network
    //#define WIFI_SSID       "NOWTVII8F7"    
//  Password
    //#define WIFI_PASSWORD   "smuqjz6jYsNW"     
//  Name of network
    #define WIFI_SSID       "SIVA_LAPTOP"    
//  Password
    #define WIFI_PASSWORD   "sivashanth"
//  Name of network
//    #define WIFI_SSID       "Orkun's Laptop"    
//  Password
//    #define WIFI_PASSWORD   "484f17Ya"     
/*  
    This code is a state mashine
    connecting when not and doing 
    nothing when connected. 
    This indicator is used to 
    determine whether it is so
*/
std::vector<float> read_cartesian = {0, 0};

bool autonomous = 1;
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
  //  {MAG, ANGLE} or (X, Y)
  std::vector<float> desired_polar      = {0, 0};
  std::vector<float> desired_cartesian  = {0, 0};
/////////////////////////////////////////////////////////////////
//  PID intance declaration
pid_ctrl_t pid;

//  Rovers displacement from optical flow sensor
float x = 0;
float y = 0;

float adjustment_angle = 0;
float initialAngle = 0;
//  parameters used in PID

std::vector<float> adjustmentVector = {0, 0};
std::vector<float> requiredVector = {0, 0};
std::vector<float> currentVector = {0, 0};

/////////////////////////////////////////////////////////////////
//  Compass readings and parameters
QMC5883LCompass compass;
#define DECLINATIONANGLE 0.483 /* * (PI / 180) */
int counter_input = 0;
float headingDegrees = 0;
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
  //sets SPI clock to 1/32 of the ESP32's clock
  SPI.setClockDivider(SPI_CLOCK_DIV32); 
  //in SPI mode 3, data is sampled on the 
  //falling edge and shifted out on the rising edge
  SPI.setDataMode(SPI_MODE3); 
  SPI.setBitOrder(MSBFIRST);


  if(code_body.mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Initializing");

    compass.read();      
    initialAngle = compass.getAzimuth();
    delay(50); //??
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
  /////////////////////////////////////////////////////////////////
  /*
      TODO: Merge the input from HTTP request to get what is my
      desired displacement, from magnitude and angle.

  /////////////////////////////////////////////////////////////////
  //  Input readings
    /*
    //  Manual joystick input readings
      read_cartesian[0] = (analogRead(XJOY))/4 - 512;
      read_cartesian[1] = (analogRead(YJOY))/4 - 512;
    */
    //  Digital joystick readings
      read_cartesian = code_body.HTTPGET();
  
    //  DESIRED x and y
    x = read_cartesian[0] / 512;
    y = read_cartesian[1] / 512;
  /////////////////////////////////////////////////////////////////////////
  //  Readings
  /*
  */
  //#if 0
  code_body.readings(
                          counter_input, 
                          compass, 
                          &headingDegrees,
                          &distance_x_OFS,
                          &distance_y_OFS,
                          &total_x1_OFS,
                          &total_y1_OFS,
                          &total_x_OFS,
                          &total_y_OFS
                          );

  /////////////////////////////////////////////////////////////////


    adjustment_angle = pid_process(&pid, adjustmentVector[0]);
    /*
    */
    adjustment_angle = (pid_process(&pid, adjustmentVector[0]));

    currentVector[0] = (!autonomous) ? (0) : sin(headingDegrees - initialAngle);
    currentVector[1] = (!autonomous) ? (0) : cos(headingDegrees - initialAngle);

  /////////////////////////////////////////////////////////////////
  //  Utilising the adjustment angles
    adjustmentVector[0] = 512 * (x - currentVector[0]); 
    adjustmentVector[1] = 512 * (y - currentVector[1]); 
    
    /*
    Serial.println("-------------");
    Serial.println(x);
    Serial.println(y);
    Serial.println("-------------");
    */

  //#else
  //if joystick's y-axis potentiometer output is low, go forward
    if(adjustmentVector[1] < -112) 
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
      speedA = map(adjustmentVector[1], -112, -512, 0, 255);
      speedB = map(adjustmentVector[1], -112, -512, 0, 255);
      Serial.println("y < -112");
    }

  //if joystick's x-axis potentiometer output is high, go backwards
    else if (adjustmentVector[1] > 188) 
    {
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
      speedA = map(adjustmentVector[1], 188, 511, 0, 255);
      speedB = map(adjustmentVector[1], 188, 511, 0, 255);
      Serial.println("y > 188");
    }

    else 
    {
      speedA = 0; speedB = 0;
      Serial.println("speed = 0");
    }

    if(adjustmentVector[0] < -112)
    {
      int XMAP = map(adjustmentVector[0], -112, -512, 0, 255);
      speedA = speedA - XMAP;
      speedB = speedB + XMAP;
      if (speedA < 0) {speedA = 0;}
      if (speedB > 255) {speedB = 255;}
      Serial.println("x < -112");
    }

    if(adjustmentVector[0] > 188)
    {
      int XMAP = map(adjustmentVector[0], 188, 511, 0, 255);
      speedA = speedA + XMAP;
      speedB = speedB - XMAP;
      if (speedA > 255) {speedA = 255;}
      if (speedB > 0) {speedB = 0;}
      Serial.println("x > 188");
    }

    if (speedA < 10) 
    {
      speedA = 0;
    }

    if (speedB < 10) 
    {
      speedB = 0;
    }

    analogWrite(PWMA, speedA);
    analogWrite(PWMB, speedB);
  /////////////////////////////////////////////////////////////////
    counter_input++;
    delay(100);
//  Serial.println("magnitude = " + String(magnitude));
  Serial.println("x = " + String(x));
  Serial.println("y = " + String(y));
  Serial.println("adjustmentVector[0] =" + String(adjustmentVector[0]));
  Serial.println("adjustmentVector[1] =" + String(adjustmentVector[1]));

  Serial.println("adjustment angle = " + String(adjustment_angle));
  Serial.println("heading_degrees = " + String(headingDegrees));

  //  #endif
  }
//  If not connected, connect 
//  and express as not connected
  if (WiFi.status() != WL_CONNECTED) 
  {
      Serial.println(".");
      //digitalWrite(LED, 
      //            !digitalRead(LED));
      delay(1000);
  }
}