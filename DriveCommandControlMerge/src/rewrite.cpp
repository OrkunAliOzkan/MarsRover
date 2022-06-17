#include <Arduino.h>
#include <SPI.h>
#include "code_body.h"
#include <Wire.h>
#include <vector>
#include "WiFi.h"
#include <Adafruit_BusIO_Register.h>

//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"
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
//  OFS to centre of rotation WAS incorrectly 144 (Centre of Mass)
  #define RADIUS 122
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
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"
//  Name of network
//   #define WIFI_SSID       "bet-hotspot"    
//  Password
//   #define WIFI_PASSWORD   "helloworld" 
//  Name of network
//    #define WIFI_SSID       "CommunityFibre10Gb_003D7"    
//  Password
//    #define WIFI_PASSWORD   "gxqxs3c3fs" 
/////////////////////////////////////////////////////////////////
// Motor controller Pins
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
/////////////////////////////////////////////////////////////////
//  Variables for OFS_Cartesian()
int prescaled_tx = 0;
int prescaled_ty = 0;
int totalpath_x_int = 0; // path in x dir of OFS - output
int totalpath_y_int = 0; // path in y dir of OFS - output
/////////////////////////////////////////////////////////////////
// Variables for OFS_Angular()
float abs_theta = 0;
float totalpath_x_flt = 0; // postscaling x displacement
float totalpath_y_flt = 0; // postscaling y displacement
/////////////////////////////////////////////////////////////////
//  Drive parameters
// starting coordinates
float A_x = 0; // TODO: Define at end of movement
float A_y = 0; // TODO: Define at end of movement

/*
OFS_Angular(
                        MD md, 
                        CURR_x, 
                        CURR_y, 
                        ABSOLUTE_ANGLE;
                        );
*/
// target coordinates
float B_x = 0;
float B_y = 0;

// current coordinates
float CURR_x = 0;
float CURR_y = 0;
float ABSOLUTE_ANGLE = 0;

int MotorSpeedA = 0; //  Final input to motors
int MotorSpeedB = 0; //  Final input to motors

int Bang_Constant = 128;  //  TODO: Map to function of choosing

bool Rot_Ctrl = 0;//  Informed by server
int arrived = 0;
int turning_arrived = 0;
/////////////////////////////////////////////////////////////////
//  Timestamp values (for server communication)
int timeStamp = 0;
int previousTimeStamp = 0;
/////////////////////////////////////////////////////////////////
// For Turning PID Controller 
float Kp_turning = 2;
float Ki_turning = 0.2;
float Kd_turning = 0.2;

float error = 0; 
float error_prev = 0;
/////////////////////////////////////////////////////////////////
//  y axis PID Variables
float error_displacement = 0;
// float pTerm_displacement = 0;
// float iTerm_displacement = 0;
// float dTerm_displacement = 0;

float Kp_displacement = 2.5;
float Ki_displacement = 0;
float Kd_displacement = 0;
/////////////////////////////////////////////////////////////////
// anglular offset PID Variables
float angular_error = 0;
float angular_error_prev = 0;
float pTerm;
float iTerm;
float dTerm;
/////////////////////////////////////////////////////////////////
float Kp = 2.5;
float Ki = 0;
float Kd = 0.05;

long currT = 0;
long prevT = 0;
float deltaT = 0;

int output;
float angle = PI/180 * 0;

MD md;
/////////////////////////////////////////////////////////////////
String post_data = "";
/////////////////////////////////////////////////////////////////
// For Automation
int automation_counter = 0;
bool automationOn = 0;
/////////////////////////////////////////////////////////////////
float displacement_y = 0;
/////////////////////////////////////////////////////////////////
/*
  UpdateRead will be used incrementally to determine if trajectory
  has changed since input. This is to prevent it from colliding w
  walls.
*/
bool UpdateRead = 0;
/////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Wire.begin();
/////////////////////////////////////////////////////////////////
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
/////////////////////////////////////////////////////////////////
  pinMode(PIN_SS,OUTPUT); //(CHIP SELECT)
  pinMode(PIN_MISO,INPUT); //(MASTER IN, SLAVE OUT)
  pinMode(PIN_MOSI,OUTPUT); //(MASTER OUT, SLAVE IN)
  pinMode(PIN_SCK,OUTPUT); //(CLOCK)
/////////////////////////////////////////////////////////////////
  SPI.begin();
  //sets SPI clock to 1/32 of the ESP32's clock
  SPI.setClockDivider(SPI_CLOCK_DIV32); 
  //in SPI mode 3, data is sampled on the 
  //falling edge and shifted out on the rising edge
  SPI.setDataMode(SPI_MODE3); 
  SPI.setBitOrder(MSBFIRST);
/////////////////////////////////////////////////////////////////
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Initializing");
/////////////////////////////////////////////////////////////////
  if(code_body.mousecam_init()==-1)
  {
    //Serial.println("Mouse cam failed to init");
    while(1);
  }
  Serial.println("Done Initializing");

  delay(3000);
  turning_arrived = 1;
  arrived = 1;
}

bool command_received = 1;

void loop()
{
    
}