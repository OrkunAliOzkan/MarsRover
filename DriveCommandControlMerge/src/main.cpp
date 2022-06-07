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

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
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
//    #define WIFI_SSID       "NOWTVII8F7"    
//  Password
//    #define WIFI_PASSWORD   "smuqjz6jYsNW"     
//  Name of network
//      #define WIFI_SSID       "SIVA_LAPTOP"    
//  Password
//      #define WIFI_PASSWORD   "sivashanth"
//  Name of network
//    #define WIFI_SSID       "Orkun's Laptop"    
//  Password
//    #define WIFI_PASSWORD   "484f17Ya" 
//  Name of network
//   #define WIFI_SSID       "bet-hotspot"    
//  Password
//   #define WIFI_PASSWORD   "helloworld" 
//  Name of network
    #define WIFI_SSID       "CommunityFibre10Gb_003D7"    
//  Password
    #define WIFI_PASSWORD   "gxqxs3c3fs" 
std::vector<float> read_cartesian = {0, 0};

String post_data = "";

bool autonomous = 1;
/////////////////////////////////////////////////////////////////
  float A_x;
  float A_y;
  float B_x;
  float B_y;
  float curr;

  float displacement = 0;
  float desiredDisplacement = 0;
  double desired_angle = 0;
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
float x_previous = 0;
float y_previous = 0;

//float steering_angle = 0;
float initialAngle = 0;
//  parameters used in PID
float magnitude = 0;

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
String lines = "---------------------------------------------------------------------";
/////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);
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


  // in setup()
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    compass.read();      
    initialAngle = compass.getAzimuth();
    delay(10); //??
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
  /////////////////////////////////////////////////////////////////
    //  Digital joystick readings
      read_cartesian = code_body.HTTPGET();
    //  DESIRED x and y
      x = read_cartesian[0];
      y = read_cartesian[1];

      desired_angle = atan2((double)y, (double)x);
  /////////////////////////////////////////////////////////////////////////
  //  Readings
  code_body.readings(
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
  if((x == x_previous) && (y == y_previous))
  {
    code_body.Brake(&speedA, &speedB);
    code_body.RotateDegrees((int)desired_angle, compass);
  }
  /////////////////////////////////////////////////////////////////
    /*
    desired_cartesian[0] = x - total_x_OFS;
    desired_cartesian[1] = y - total_y_OFS;
    magnitude = sqrt(
                      (desired_cartesian[0] * desired_cartesian[0]) + 
                      (desired_cartesian[1] * desired_cartesian[1])
                    );

    currentVector[1] = (magnitude / 2) * cos(headingDegrees - initialAngle);
    currentVector[0] = (magnitude / 2) * sin(headingDegrees - initialAngle);
  /////////////////////////////////////////////////////////////////
  //  Utilising the adjustment angles System is absolute, not relative

    adjustmentVector[0] = (desired_cartesian[0] - currentVector[0]); 
    adjustmentVector[1] = (desired_cartesian[1] - currentVector[1]); 

    steering_angle = pid_process(&pid, adjustmentVector[0]);

    adjustmentVector[0] *= (sin(steering_angle)); 
    adjustmentVector[1] *= (cos(steering_angle)); */
/*
    Serial.println("---------------------------------------------------------------------");
    Serial.println("x = " + String(x));
    Serial.println("y = " + String(y));
    Serial.println("adjustmentVector[0] =" + String(adjustmentVector[0]));
    Serial.println("adjustmentVector[1] =" + String(adjustmentVector[1]));
    Serial.println("currentVector[0] =" + String(currentVector[0]));
    Serial.println("currentVector[1] =" + String(currentVector[1]));
    Serial.println("desired_cartesian[0] =" + String(desired_cartesian[0]));
    Serial.println("desired_cartesian[1] =" + String(desired_cartesian[1]));
    Serial.println("adjustment angle = " + String(steering_angle));
    Serial.println("heading_degrees = " + String(headingDegrees));
    Serial.println("total_x_OFS = " + String(total_x_OFS));
    Serial.println("total_y_OFS = " + String(total_y_OFS));
    Serial.println("---------------------------------------------------------------------");
*/
    post_data = lines + 
          "\n" + 
          ("x = " + String(x)) + 
          "\n" + 
          ("y = " + String(y)) + 
          "\n" + 
          ("heading_degrees = " + String(headingDegrees)) + 
          "\n" + 
          ("total_x_OFS = " + String(total_x_OFS)) + 
          "\n" + 
          ("total_y_OFS = " + String(total_y_OFS)) + 
          "\n" + 
          lines + 
          "\n";
    //if joystick's y-axis potentiometer output is high, go forward
    //("adjustmentVector[0] =" + String(adjustmentVector[0])) + "\n" + ("adjustmentVector[1] =" + String(adjustmentVector[1])) + "\n" + ("currentVector[0] =" + String(currentVector[0])) + "\n" + ("currentVector[1] =" + String(currentVector[1])) + "\n" + ("desired_cartesian[0] =" + String(desired_cartesian[0])) + "\n" + ("desired_cartesian[1] =" + String(desired_cartesian[1])) + "\n" + ("adjustment angle = " + String(steering_angle)) + "\n" + 
    /////////////////////////////////////////////////////////////////
    displacement =        sqrt(pow(total_x_OFS - A_x, 2) + pow(total_y_OFS - A_y, 2));
    desiredDisplacement = sqrt(pow(B_x - A_x, 2) + pow(B_y - A_y, 2));
    if(displacement == desiredDisplacement)
    {
      code_body.Brake(&speedA, &speedB);
    }
    else if(displacement > desiredDisplacement)
    {
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
      speedA = 128;
      speedA = 128;
    }
    else
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
      speedA = 255;
      speedB = 255;
    }
    /////////////////////////////////////////////////////////////////
    analogWrite(PWMA, speedA);
    analogWrite(PWMB, speedB);
    delay(10);
  /////////////////////////////////////////////////////////////////
    counter_input++;
    delay(10);
  }
//  If not connected, connect and express as not connected
  if (WiFi.status() != WL_CONNECTED) 
  {
      Serial.println(".");
      delay(1000);
  }
  x_previous = x;
  y_previous = y;
  code_body.HTTPPOST(post_data);
}