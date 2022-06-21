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

void OFS_Squal(MD md)
{
    //  Optical sensor readings
        mousecam_read_motion(&md);
        delay(1000);

        Serial.println("Number of stars = " + String((md.squal)/4));
      }

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
  if(mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }
/////////////////////////////////////////////////////////////////
 
  delay(5000);
}

void loop()
{  
    OFS_Squal(md);
}
