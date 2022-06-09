#include <SPI.h>
#include <PID_v1.h>
#include <Wire.h>

#define RADIUS 144
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
/////////////////////////////////////////////////////////////////
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
/////////////////////////////////////////////////////////////////
//  Optical Flow Sensor parameters
int distance_x = 0; //motion dx
int distance_y = 0; //motion dy

int total_x1 = 0; //prescaling x displacement
int total_y1 = 0; //prescaling y displacement

struct MD
{
 byte motion;
 char dx, dy;
 byte squal;
 word shutter;
 byte max_pix;
};

MD md;
/////////////////////////////////////////////////////////////////
//  Drive parameters
double A_x = 0;
double A_y = 0;

double B_x = 0;
double B_y = 500;

int CURR_x = 0;
int CURR_y = 0;

int beginingRotationX = 0;
float angle = (0)* PI/180;     // Angle offset by, from server
int PID_speed = 0;   // PID output
int PWM_offset = 0;  // Mapping output

int MotorSpeedA = 178; //  Final input to motors
int MotorSpeedB = 178; //  Final input to motors

bool NeedToRotate = 0;//  Informed by server

/////////////////////////////////////////////////////////////////
//  Functions
int convTwosComp(int b)
{
    //Convert from 2's complement
    if(b & 0x80)
    {
      b = -1 * ((b ^ 0xff) + 1);
    }
    return b;
}

void mousecam_reset() //reset procedure = set reset high, followed by set reset low
{
    digitalWrite(PIN_MOUSECAM_RESET,HIGH);
    delay(1); // reset pulse >10us (constraint given in data sheet)
    digitalWrite(PIN_MOUSECAM_RESET,LOW);
    delay(35); // 35ms from reset to functional
}

int mousecam_init() //initialisation procedure
{
    pinMode(PIN_MOUSECAM_RESET,OUTPUT);
    pinMode(PIN_MOUSECAM_CS,OUTPUT);
    digitalWrite(PIN_MOUSECAM_CS,HIGH);
    mousecam_reset();
    return 1;
}

void mousecam_write_reg(int *reg, int *val) //write to mouse sensor's register 
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(*reg | 0x80); //address of register to write to sensor's register | buffer size?
    SPI.transfer(*val); //data to write to sensor's register
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(50); //required wait time
}

void mousecam_read_motion(struct MD *p)
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(ADNS3080_MOTION_BURST); //reading from motion burst register == activation of Motion Read mode
    delayMicroseconds(75);
    p->motion =  SPI.transfer(0xff); //the sensor responds with the structure MD's data, in the given order
    p->dx =  SPI.transfer(0xff);
    p->dy =  SPI.transfer(0xff);
    p->squal =  SPI.transfer(0xff);
    p->shutter =  SPI.transfer(0xff)<<8;
    p->shutter |=  SPI.transfer(0xff);
    p->max_pix =  SPI.transfer(0xff);
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(5); //necessary wait time
}

void OpticalFlowSensorReadings(MD md,int * distance_x,int * distance_y,int * total_x1,int * total_y1, int * total_x, int * total_y)
{
  //  Optical sensor readings
  mousecam_read_motion(&md);
  delay(100);
  
  *distance_y = convTwosComp(md.dy);
  *distance_x = convTwosComp(md.dx);
  *total_x1 = *total_x1 + *distance_x;
  *total_y1 = *total_y1 + *distance_y;
  
  *total_x = *total_x1/4.95;
  *total_y = *total_y1/4.95;
  Serial.println("Dy: " + String(*distance_y));
  Serial.println("Dx: " + String(*distance_x));
  Serial.println("Total y: " + String(*total_x));
  Serial.println("Total x: " + String(*total_y));
}
/////////////////////////////////////////////////////////////////
//  Turning PID control parameters
double Turning_Setpoint, Turning_Input, Turning_Output;

//Specify the turning PID's links and initial tuning parameters
PID TurningPID(&Turning_Input, &Turning_Output, &Turning_Setpoint,0,0,0, DIRECT);

//  Driving PID control parameters
double Driving_Setpoint;
double Driving_Output;
double error;
double old_error;
long currT = 0;
long prevT = 0;
float deltaT = 0;
float integralterm = 0;
float derivativeterm = 0;
float Kp = 0.1;
float Ki = 0.1;
float Kd = 0.02;
int arrived;

//Specify the driving PID's links and initial tuning parameters //P,I,D
PID DrivingPID(&error, &Driving_Output, &Driving_Setpoint,0.33,0,0, DIRECT);
/////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  Driving_Setpoint = 0;
  //turn the PID on
  TurningPID.SetMode(AUTOMATIC);
  DrivingPID.SetMode(AUTOMATIC);
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
  if(mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }
/////////////////////////////////////////////////////////////////
  OpticalFlowSensorReadings(md,&distance_x,&distance_y,&total_x1,&total_y1,&CURR_x,&CURR_y);
  beginingRotationX = CURR_x;
/////////////////////////////////////////////////////////////////
 
  delay(5000);
  // initial conditions
  old_error = CURR_x - A_x;
  arrived = 0;
}

void loop()
{  
//  //  Server input
//  //B = {0, 512};
//  delayMicroseconds(10);
//  if(NeedToRotate)
//  {
//    //  Rotate
//    OpticalFlowSensorReadings(md,&distance_x,&distance_y,&total_x1,&total_y1,&CURR_x,&CURR_y);
//    if(CURR_x - beginingRotationX == RADIUS*angle)
//    {
//      NeedToRotate = 0;
//    }
//    else 
//    {
//      Turning_Setpoint = RADIUS*angle;
//      Turning_Input = CURR_x - beginingRotationX;
//      TurningPID.Compute();
//      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  
//      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
//      analogWrite(PWMA, Turning_Output);  //  TODO: See if mapping works
//      analogWrite(PWMB, Turning_Output);
//    } 
//  }
//  else
//  {
//    if (!arrived) {
//      //  Not need to rotate
//      OpticalFlowSensorReadings(md,&distance_x,&distance_y,&total_x1,&total_y1,&CURR_x,&CURR_y);
//      currT = micros();
//      deltaT = ((float) (currT-prevT))/1.0e6;
//      prevT = currT;
//
//      // controller
//      error = CURR_x - A_x;
//      integralterm = integralterm + (error*deltaT);
//      derivativeterm = (error - old_error)/deltaT;
//      Driving_Output = (error * Kp) + (integralterm * Ki) + (derivativeterm * Kd); //0.3 is good, 0.33 decent
//      //PWM_offset = map(abs(Driving_Output), 0, 1000, 0, 255); 
//      
//      
//      MotorSpeedA += Driving_Output;
//      MotorSpeedB -= Driving_Output;
//      
//      if (MotorSpeedA < 100) {MotorSpeedA = 100;}
//      if (MotorSpeedA > 255) {MotorSpeedA = 255;}
//      
//      if (MotorSpeedB < 100) {MotorSpeedB = 100;}
//      if (MotorSpeedB > 255) {MotorSpeedB = 255;}
//      
//      if ((CURR_y - B_y < 10) && (CURR_y - B_y > -10)) 
//      {
//        arrived = 1;
//      }
//      
//      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
//      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
//      analogWrite(PWMA, MotorSpeedA);  //  TODO: See if mapping works
//      analogWrite(PWMB, MotorSpeedB);
//      
//      Serial.println();
//      //Serial.println("Absolute Error at end: " + String(deltaT));
//      
//      old_error = error;
//      Serial.println("Drive output: " + String(Driving_Output));
//      Serial.println("MotorSpeedA: " + String(MotorSpeedA));
//      Serial.println("MotorSpeedB: " + String(MotorSpeedB));
//      Serial.println("CURR_x: " + String(CURR_x));
//      Serial.println("CURR_y: " + String(CURR_y));
//      Serial.println("Error: " + String(error));
//    };
//  }
  OpticalFlowSensorReadings(md,&distance_x,&distance_y,&total_x1,&total_y1,&CURR_x,&CURR_y);
//  Serial.println("CURR_x: " + String(CURR_x));
//  Serial.println("CURR_y: " + String(CURR_y));
}
