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
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
/////////////////////////////////////////////////////////////////
//  Optical Flow Sensor parameters
int prescaled_tx = 0;
int prescaled_ty = 0;
int totalpath_x_int = 0; // prescaling x displacement
int totalpath_y_int = 0; // prescaling y displacement
/////////////////////////////////////////////////////////////////
//  Drive parameters
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

float B_x = 0;
float B_y = 750;

float CURR_x = 0;
float CURR_y = 75;
float ABSOLUTE_ANGLE = 0;

int MotorSpeedA = 0; //  Final input to motors
int MotorSpeedB = 0; //  Final input to motors

int Bang_Constant = 128;  //  TODO: Map to function of choosing
/////////////////////////////////////////////////////////////////
//  Timestamp values
  int timeStamp = 0;
  int previousTimeStamp = 0;
/////////////////////////////////////////////////////////////////
float error = 0; 
float error_prev = 0;
bool Rot_Ctrl = 0;//  Informed by server
int arrived = 0;
int turning_arrived = 0;
/////////////////////////////////////////////////////////////////
float abs_theta = 0;
float totalpath_x_flt = 0; // postscaling x displacement
float totalpath_y_flt = 0; // postscaling y displacement
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
//  pivoting PID Variables
float Kp_turning = 2;
float Ki_turning = 0.2;
float Kd_turning = 0.2;

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
std::vector<float> read_cartesian;
/////////////////////////////////////////////////////////////////
int automation_counter = 0;
bool automationOn = 1;
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

  delay(3000);
  turning_arrived = 1;
  arrived = 1;
}

bool command_received = 1;
void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
      
      timeStamp = millis();
      if(timeStamp - previousTimeStamp > 200)
      {
        previousTimeStamp = timeStamp;
        UpdateRead = 1;
      }
        /////////////////////////////////////////////////////////////////
        if ((turning_arrived && arrived) || (UpdateRead))
        {
            //read_cartesian = code_body.HTTPGET();
            ////code_body.HTTPPOST(post_data);
            //angle = read_cartesian[0] / 180 * PI;
            //B_y = read_cartesian[1];

            ///////////////////////////////////////
          //  Modified
            A_x = B_x;
            A_y = B_y;


            if(automationOn)
            {
              code_body.automation(
              &automation_counter,
              232, 1050,
              3, 2,
              A_x, A_y,
              &B_x, &B_y, &angle
              );
            }
            else{
              /*
                Some type of get request and post
              */
            }

            displacement_y = sqrt(
                            pow((B_y - A_y), 2) + 
                            pow((B_x - A_x), 2)
                            );
            ///////////////////////////////////////
            turning_arrived = 0;
            arrived = 0;
            UpdateRead = 0;
        }
        /////////////////////////////////////////////////////////////////////////
        if (command_received) 
        {  
            if(!turning_arrived)
            {
                //  Rotate
                code_body.OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
                //code_body.OFS_Angular(md, &CURR_x, &CURR_y, &ABSOLUTE_ANGLE);
                error = (totalpath_x_int - RADIUS*angle);
                currT = micros();
                deltaT = ((float) (currT-prevT))/1.0e6;
                prevT = currT;

                //    error = (abs(error) < 10) ? ( (error > 0) ? (15) : (-15) ) : (error);
                //    error = (abs(totalpath_x_int - RADIUS*angle) < 5) ? (0) : (error);

                pTerm = error;
                //    iTerm += (error * deltaT);
                //    Serial.println("before shit");
                //    dTerm = (error - error_prev)/deltaT;
                //    Serial.println("avoid shit");
                output = Kp_turning * pTerm;
                error_prev = error;

                post_data +=("-------------------------------------------------------");
                post_data += "\n";
                post_data +=("displacement_y: " + String(displacement_y));
                post_data += "\n";
                post_data +=("angle: " + String(angle));
                post_data += "\n";
                post_data +=("deltaT: " + String(deltaT));
                post_data += "\n";
                post_data +=("P: " + String(pTerm * Kp_turning));
                post_data += "\n";
                post_data +=("I: " + String(iTerm * Ki_turning));
                post_data += "\n";
                post_data +=("D: " + String(dTerm * Kd_turning));
                post_data += "\n";
                post_data +=("error: " + String(error));
                post_data += "\n";
                post_data +=("output: " + String(output));
                post_data += "\n";
                post_data +=("TOTAL_PATH_x: " + String(totalpath_x_int));
                post_data += "\n";
                output = abs(output);

                if (error <= 0)
                {
                    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
                    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
                }
                else
                {
                    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
                    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
                }

                output = abs(output);
                if (abs(error) < 3)
                {

                    turning_arrived = 1; 
                    output = 0; 
                    post_data += ("Stopped");
                    prevT = 0;
                    pTerm = 0;
                    iTerm = 0;
                    dTerm = 0;

                    A_x = CURR_x;
                    A_y = CURR_y;
                } 
                else if (output > 255) 
                {
                    output = 255;
                }
                else if (output < 38) 
                {
                    output = 38;
                }

                analogWrite(PWMA, output);  //  TODO: See if mapping works
                analogWrite(PWMB, output);
            } 
            else if (!arrived && turning_arrived) {
                //  Straight
                code_body.OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
                //code_body.OFS_Angular(md, &CURR_x, &CURR_y,  &ABSOLUTE_ANGLE);
                //code_body.x_displacement(&CURR_x,&CURR_y,&A_x,&A_y,&B_x,&displacement_y,&angular_error);

                currT = micros();

                deltaT = ((float) (currT-prevT))/1.0e6;
                prevT = currT;

                // controller
                angular_error = (totalpath_x_int);
                
                pTerm = (angular_error);
                iTerm += (angular_error*deltaT);
                //    dTerm = (angular_error - angular_error_prev)/deltaT;
                output = pTerm * Kp + iTerm * Ki; //0.3 is good, 0.33 decent

                angular_error_prev = angular_error;
                post_data +=("Angular Error: " + String(angular_error));
                post_data += "\n";
                post_data +=("Output: " + String(output));
                post_data += "\n";
                post_data +=("TOTAL_PATH_x: " + String(totalpath_x_int));
                post_data += "\n";
                post_data +=("TOTAL_PATH_y: " + String(totalpath_y_int));
                post_data += "\n";
                //output = abs(output);

                {
                    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CW  // ACW Rover
                    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CCW
                }
                //////////////////////////////////////////////////////////////////////////////////////////////
                //  y-axis p controller
                error_displacement = displacement_y - totalpath_y_int;
                Bang_Constant = Kp_displacement * error_displacement;
                Bang_Constant = (Bang_Constant > 220) ? (220) : (Bang_Constant);
                Bang_Constant = (Bang_Constant < 30) ? (30) : (Bang_Constant);
                //////////////////////////////////////////////////////////////////////////////////////////////
                //output = abs(output);
                MotorSpeedA = Bang_Constant + output;
                MotorSpeedB = Bang_Constant - output;

                if (MotorSpeedA < 0) {MotorSpeedA = 0;}
                if (MotorSpeedA > 255) {MotorSpeedA = 255;}

                if (MotorSpeedB < 0) {MotorSpeedB = 0;}
                if (MotorSpeedB > 255) {MotorSpeedB = 255;}

                if ((totalpath_y_int - displacement_y < 10) && (totalpath_y_int - displacement_y > -10)) 
                {
                    arrived = 1;
                    MotorSpeedA = 0;
                    MotorSpeedB = 0;

                }
                post_data +=("-------------------------------------------------------");
                post_data += "CURR_x:\t" + String(CURR_x);
                post_data += "\n";
                post_data += "CURR_y:\t" + String(CURR_y);
                post_data += "\n";
                post_data += "A_x:\t" + String(A_x);
                post_data += "\n";
                post_data += "A_y:\t" + String(A_y);
                post_data += "\n";
                post_data += "B_x:\t" + String(B_x);
                post_data += "\n";
                post_data += "displacement_y:\t" + String(displacement_y);
                post_data += "\n";
                post_data += "ABSOLUTE_ANGLE:\t" + String(ABSOLUTE_ANGLE);
                post_data += "\n";
                post_data +=("MotorSpeedA: " + String(MotorSpeedA));
                post_data += "\n";
                post_data +=("MotorSpeedB: " + String(MotorSpeedB));
                post_data += "\n";

                analogWrite(PWMA, MotorSpeedA);  //  TODO: See if mapping works
                analogWrite(PWMB, MotorSpeedB);    
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////
    //  If not connected, connect and express as not connected
    if (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
    }
}