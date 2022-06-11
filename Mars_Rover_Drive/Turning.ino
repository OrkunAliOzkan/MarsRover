#include <SPI.h>
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
struct GYRO
{
 char dx, dy, dz;

};
GYRO gyro;
/////////////////////////////////////////////////////////////////
//  Optical Flow Sensor parameters
int distance_x = 0; // motion dx
int distance_y = 0; // motion dy
int total_x1 = 0; // prescaling x displacement
int total_y1 = 0; // prescaling y displacement
int TOTAL_PATH_x1 = 0; // postscaling x displacement
int TOTAL_PATH_y1 = 0; // postscaling y displacement
/////////////////////////////////////////////////////////////////
//  Drive parameters
double A_x = 0;
double A_y = 0;

double B_x = 0;
double B_y = 500;

int TOTAL_PATH_x = 0;
int TOTAL_PATH_y = 0;

int MotorSpeedA = 0; //  Final input to motors
int MotorSpeedB = 0; //  Final input to motors

int Bang_Constant = 128;
/////////////////////////////////////////////////////////////////
//  Computing angle parameters
int *vector_a_x = 0;
int *vector_a_y = 0;
int *vector_b_x = 0;
int *vector_b_y = 0;
int *mag_a = 0;
int *mag_b = 0;
int *dot_product, = 0;
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
//  Serial.println("Dy: " + String(*distance_y));
//  Serial.println("Dx: " + String(*distance_x));
//  Serial.println("Total y: " + String(*total_y));
//  Serial.println("Total x: " + String(*total_x));
}

void GyroScopeSensorReadings(struct GYRO *gyro)
{
    /*
    TODO: Need to know lib to use, waiting for Siva for this one.
    */
}

void DeterminePosition(struct GYRO *gyro, int *TOTAL_PATH_x, int *TOTAL_PATH_y, int *myX, int *myY)
{
    /*
    TODO: Insert Bryans function for finding position

    Firstly determine the angle along x axis:
    
    */

}

void ErrorAngleComputation(
                        int *vector_a_x, int *vector_a_y, int *vector_b_x, int *vector_b_y, 
                        int *mag_a, int *mag_b, int *dot_product,  
                        int *error, int *CURR_x, int *CURR_y, int *A_x, int *A_y, int *B_x, int *B_y)
{
/*
    This is to determine the angle of the controller.
    Angle between points current pos, destination and start point
*/

    vector_a_x = (B_x - CURR_x)
    vector_a_y = (B_y - CURR_y)

    vector_b_x = (A_x - CURR_x)
    vector_b_y = (A_y - CURR_y)

    mag_a = sqrt( pow((B_x - CURR_x), 2) + pow((B_y - CURR_y), 2))
    mag_b = sqrt( pow((A_x - CURR_x), 2) + pow((A_y - CURR_y), 2))

    dot_product = (B_x - CURR_x)*(A_x - CURR_x) + (B_y - CURR_y)*(A_y - CURR_y)
    error = acos(dot_product/(mag_a*mag_b));
}
/////////////////////////////////////////////////////////////////
int error = 0; 
bool Rot_Ctrl = 0;//  Informed by server
int arrived = 0;
/////////////////////////////////////////////////////////////////
// PID Variables
int angular_error = 0;
int angular_error_x = 0;
int angular_error_prev = 0;
int pTerm_x;
int iTerm_x;
int dTerm_x;

int pTerm_y;
int iTerm_y;
int dTerm_y;

float Kp_x = 0;
float Ki_x = 0;
float Kd_x = 0;

float Kp_y = 2;
float Ki_y = 0.05;
float Kd_y = 0;

long currT = 0;
long prevT = 0;
long deltaT = 0;
int output;
int BangTerm;
float angle = PI/180 * 0;
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
  SPI.setClocKd_xivider(SPI_CLOCK_DIV32); 
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
OpticalFlowSensorReadings(md, &distance_x, &distance_y, &total_x1, &total_y1,  &TOTAL_PATH_x,  &TOTAL_PATH_y);
/////////////////////////////////////////////////////////////////
 
  delay(5000);
}

void loop()
{  
  if(Rot_Ctrl)
  {
    //  Rotate
    OpticalFlowSensorReadings(md, &distance_x, &distance_y, &total_x1, &total_y1, &TOTAL_PATH_x, &TOTAL_PATH_y);
    error = ; /*    Displacement from B     */
    pTerm_x = Kp_x * error;
    
    output = pTerm_x;

    Serial.println("error: " + String(error));
    Serial.println("output: " + String(output));
    Serial.println("TOTAL_PATH_x: " + String(TOTAL_PATH_x));

    output = abs(output);
    if(error <= 0)
    {
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
    }
    else
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
    }
    
    //output = abs(output);
    if (output > 255) {
      output = 255;
    }
    else if (output < 0) {
      output = 0;
    }
    
    analogWrite(PWMA, output);  //  TODO: See if mapping works
    analogWrite(PWMB, output);
  } else if (!arrived) {
    //  Readings
    OpticalFlowSensorReadings(md, &distance_x, &distance_y, &total_x1, &total_y1, &TOTAL_PATH_x, &TOTAL_PATH_y);
    currT = micros();
    deltaT = ((float) (currT-prevT))/1.0e-6;
    prevT = currT;

    // controller (y)
    angular_error = (TOTAL_PATH_x - RADIUS*angle);
    pTerm_y = (angular_error * Kp_y);
    iTerm_y = 0;    //iTerm_y + (angular_error*deltaT);
    dTerm_y = 0;    //(angular_error - angular_error_prev)/deltaT;
    BangTerm = pTerm_y + (iTerm_y * Ki_y) + (dTerm_y * Kd_y); //0.3 is good, 0.33 decent

    //  contriller (X) 
    /*
        TODO: Have such that it takes the readings and computes angles
    */
    GyroScopeSensorReadings(&gyro);                         //  TODO: Get Siva to lyk lib to use...
    DeterminePosition(&gyro, &TOTAL_PATH_x, &TOTAL_PATH_y, &myX, &myY); //  TODO: What the fuck
    ErrorAngleComputation(&angular_error_x, &myX, &myY, &A_x, &A_y, &B_x, &B_y);
    pTerm_x = (angular_error_x * Kp_x);
    iTerm_x = iTerm_x + (angular_error_x*deltaT);
    dTerm_x = (angular_error_x - angular_error_x)/deltaT;
    output = pTerm_x + (iTerm_x * Ki_x) + (dTerm_x * Kd_x); //0.3 is good, 0.33 decent

    Serial.println();
    Serial.println("Angular Error: " + String(angular_error));
    Serial.println("Angular Error x: " + String(angular_error_x));
    Serial.println("Output: " + String(output));
    Serial.println("Output_y: " + String(output_y));
    Serial.println("TOTAL_PATH_x: " + String(TOTAL_PATH_x));
    Serial.println("TOTAL_PATH_y: " + String(TOTAL_PATH_y));

    //output = abs(output);
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CW  // ACW Rover
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CCW
    }

    //output = abs(output);
    MotorSpeedA = Bang_Constant + output;
    MotorSpeedB = Bang_Constant - output;
    
    if (MotorSpeedA < 0) {MotorSpeedA = 0;}
    if (MotorSpeedA > 255) {MotorSpeedA = 255;}
    
    if (MotorSpeedB < 0) {MotorSpeedB = 0;}
    if (MotorSpeedB > 255) {MotorSpeedB = 255;}

    if ((TOTAL_PATH_y - B_y < 10) && (TOTAL_PATH_y - B_y > -10)) 
    {
        arrived = 1;
        MotorSpeedA = 0;
        MotorSpeedB = 0;
    }
    Serial.println("MotorSpeedA: " + String(MotorSpeedA));
    Serial.println("MotorSpeedB: " + String(MotorSpeedB));
    analogWrite(PWMA, MotorSpeedA);  //  TODO: See if mapping works
    analogWrite(PWMB, MotorSpeedB);    
  }
}