#include <Arduino.h>
#include "SPI.h" //https://www.arduino.cc/reference/en/language/functions/communication/spi/

// these pins may be different on different boards

#define PIN_SS        5 //slave select/chip select (ESP32 pin number)
#define PIN_MISO      19 //output data from mouse sensor (ESP32 pin number)
#define PIN_MOSI      23 //input data to mouse sensor (ESP32 pin number)
#define PIN_SCK       18 //synchronous clock (ESP32 pin number)

#define PIN_MOUSECAM_RESET     35 //reset (ESP32 pin number)
#define PIN_MOUSECAM_CS        5 //chip select (ESP32 pin number again)

#define ADNS3080_PIXELS_X                 30 //constant for x-axis pixel count 
#define ADNS3080_PIXELS_Y                 30 //constant for y-axis pixel count 

//list of registers on mouse sensor IC (addresses where respective data is stored)
#define ADNS3080_PRODUCT_ID            0x00 //read  |
#define ADNS3080_REVISION_ID           0x01 
#define ADNS3080_MOTION                0x02
#define ADNS3080_DELTA_X               0x03
#define ADNS3080_DELTA_Y               0x04
#define ADNS3080_SQUAL                 0x05
#define ADNS3080_PIXEL_SUM             0x06
#define ADNS3080_MAXIMUM_PIXEL         0x07
//0x08 and 0x09 are reserved
#define ADNS3080_CONFIGURATION_BITS    0x0a //read and write |
#define ADNS3080_EXTENDED_CONFIG       0x0b
#define ADNS3080_DATA_OUT_LOWER        0x0c //read |
#define ADNS3080_DATA_OUT_UPPER        0x0d
#define ADNS3080_SHUTTER_LOWER         0x0e
#define ADNS3080_SHUTTER_UPPER         0x0f
#define ADNS3080_FRAME_PERIOD_LOWER    0x10
#define ADNS3080_FRAME_PERIOD_UPPER    0x11
#define ADNS3080_MOTION_CLEAR          0x12 //write
#define ADNS3080_FRAME_CAPTURE         0x13 //read and write
#define ADNS3080_SROM_ENABLE           0x14 //write
//0x15 to 0x18 are reserved
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER      0x19 //read and write |
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER      0x1a
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER      0x1b
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER      0x1c
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER           0x1e
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER           0x1e
#define ADNS3080_SROM_ID               0x1f //read 
//0x20 to 0x3c are reserved
#define ADNS3080_OBSERVATION           0x3d //read and write
//0x3e is reserved
#define ADNS3080_INVERSE_PRODUCT_ID    0x3f //read |
#define ADNS3080_PIXEL_BURST           0x40 
#define ADNS3080_MOTION_BURST          0x50
#define ADNS3080_SROM_LOAD             0x60 //write
#define ADNS3080_PRODUCT_ID_VAL        0x17

//////////////////////////////////////////////////////////////////////DEFINITIONS////////////////
int total_x_optical = 0; //initialises actual total x-displacement from starting/reset position 
int total_y_optical = 0; //initialises actual total y-displacement from starting/reset position 


int total_x_optical1 = 0; //total x displacement before applying scale factor
int total_y_optical1 = 0; //total y displacement before applying scale factor


int x_optical=0;
int y_optical=0;

int a=0;
int b=0;

int distance_x_optical=0;
int distance_y_optical=0;

//volatile means that it can be changed by some external means which the compiler 
//isn't currently aware of; therefore tells it to not optimize/mess around with involved code
volatile byte movementflag=0; 
volatile int xy_optical_dat[2]; 


int convTwosComp(int b){
  //Convert from 2's complement
  if(b & 0x80){
    b = -1 * ((b ^ 0xff) + 1);
    }
  return b;
  }


int tdistance = 0;



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
  return 1 ;
}

void mousecam_write_reg(int reg, int val) //write to mouse sensor's register 
{
  digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
  SPI.transfer(reg | 0x80); //address of register to write to sensor's register | buffer size?
  SPI.transfer(val); //data to write to sensor's register
  digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
  delayMicroseconds(50); //required wait time
}

int mousecam_read_reg(int reg) //read from mouse sensor's register
{
  digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
  SPI.transfer(reg); ///address of register to read from sensor's register
  delayMicroseconds(75);
  int ret = SPI.transfer(0xff);
  digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
  delayMicroseconds(1);
  return ret; //value read from register
}

struct MD
{
 byte motion;
 char dx_optical, dy_optical;
 byte squal;
 word shutter;
 byte max_pix_optical;
};


void mousecam_read_motion(struct MD *p)
{
  digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
  SPI.transfer(ADNS3080_MOTION_BURST); //reading from motion burst register == activation of Motion Read mode
  delayMicroseconds(75);
  p->motion =  SPI.transfer(0xff); //the sensor responds with the structure MD's data, in the given order
  p->dx_optical =  SPI.transfer(0xff);
  p->dy_optical =  SPI.transfer(0xff);
  p->squal =  SPI.transfer(0xff);
  p->shutter =  SPI.transfer(0xff)<<8;
  p->shutter |=  SPI.transfer(0xff);
  p->max_pix_optical =  SPI.transfer(0xff);
  digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
  delayMicroseconds(5); //necessary wait time
}

// pdata must point to an array of size ADNS3080_PIXELS_X x ADNS3080_PIXELS_Y
// you must call mousecam_reset() after this (Frame Capture) if you want to go back to normal operation

int mousecam_frame_capture(byte *pdata) //the next available roughly (5/3) frames (1536 values) get stored to memory
//then data is retrieved by reading the Pixel Burst register once using normal read
{
  mousecam_write_reg(ADNS3080_FRAME_CAPTURE,0x83);

  digitalWrite(PIN_MOUSECAM_CS, LOW);

  SPI.transfer(ADNS3080_PIXEL_BURST);
  delayMicroseconds(50);

  int pix_optical;
  byte started = 0;
  int count;
  int timeout = 0;
  int ret = 0;
  for(count = 0; count < ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y; )
  {
    pix_optical = SPI.transfer(0xff);
    delayMicroseconds(10);
    if(started==0)
    {
      if(pix_optical&0x40)
        started = 1;
      else
      {
        timeout++;
        if(timeout==100)
        {
          ret = -1;
          break;
        }
      }
    }
    if(started==1)
    {
      pdata[count++] = (pix_optical & 0x3f)<<2; // scale to normal grayscale byte range
    }
  }

  digitalWrite(PIN_MOUSECAM_CS,HIGH); //terminates Burst Mode
  delayMicroseconds(14); //wait for t_bexit

  return ret;
}

////////////////////////////////////////////////////////////////////ARDUINO SETUP////////////////////////////////////
////////////////////////////////////initialises pin names, variables, functions and applies initial values///////////
void setup()
{
  pinMode(PIN_SS,OUTPUT); //sets the pin as an output pin
  pinMode(PIN_MISO,INPUT); //sets the pin as an input pin
  pinMode(PIN_MOSI,OUTPUT); //sets the pin as an output pin
  pinMode(PIN_SCK,OUTPUT); //sets the pin as an output pin

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32); //sets SPI clock to 1/32 of the ESP32's clock
  SPI.setDataMode(SPI_MODE3); //in SPI mode 3, data is sampled on the falling edge and shifted out on the rising edge
  SPI.setBitOrder(MSBFIRST);

  Serial.begin(9600); //begins serial communication between the ESP32 and the other device

  if(mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }
}

char asciiart(int k)
{
  static char foo[] = "WX86*3I>!;~:,`. ";
  return foo[k>>4];
}

byte frame[ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y];

/////////////////////////////////////////////////////////////////////ARDUINO CONTINOUS LOOP///////////////////////////

void loop()
{
 // #if 0   is a pre-processor directive; 
 // it is used to comment out/ignore a large 
 // block of code (until else) -> i.e. do if 0=/=0
 #if 0 

  /*
    if(movementflag){

    tdistance = tdistance + convTwosComp(xydat[0]);
    Serial.println("Distance = " + String(tdistance));
    movementflag=0;
    delay(3);
    }

  */

  // if enabled, the section below, grabs frames and outputs them as ASCII art

  if(mousecam_frame_capture(frame)==0)
  {
    int i,j,k;
    for(i=0, k=0; i<ADNS3080_PIXELS_Y; i++)
    {
      for(j=0; j<ADNS3080_PIXELS_X; j++, k++)
      {
        Serial.print(asciiart(frame[k]));
        Serial.print(' ');
      }
      Serial.println();
    }
  }
  Serial.println();
  delay(250);

  #else

  // if enabled, the section below produces a bar graph (***********) 
  //of the surface quality (SQUAL) divided by factor of 4, that can be used to focus the camera
  // also drawn is the average pixel value 0-63, the shutter speed and the motion dx,dy.

  int val = mousecam_read_reg(ADNS3080_PIXEL_SUM);
  MD md;
  mousecam_read_motion(&md);
  /////////////////FOR LOOP//////////////////////
  for(int i=0; i<md.squal/4; i++)
    Serial.print('*');
  Serial.print(' ');
  Serial.print((val*100)/351);
  Serial.print(' ');
  Serial.print(md.shutter); Serial.print(" (");
  Serial.print((int)md.dx_optical); Serial.print(',');
  Serial.print((int)md.dy_optical); Serial.println(')');

  // Serial.println(md.max_pix_optical);
  delay(100);


    distance_x_optical = md.dx_optical; //convTwosComp(md.dx_optical);
    distance_y_optical = md.dy_optical; //convTwosComp(md.dy);

total_x_optical1 = total_x_optical1 + distance_x_optical;
total_y_optical1 = total_y_optical1 + distance_y_optical;

//!!!!CHECK SIGNIFICANCE OF 157 WITH GTA!!!!! 
//possibly scaling from mouse's perceived motion, 
//to actual motion (cm) (accounting for distance of sensor from ground)
total_x_optical = total_x_optical1/157; 
total_y_optical = total_y_optical1/157;


Serial.print('\n');


Serial.println("Distance_x = " + String(total_x_optical));

Serial.println("Distance_y = " + String(total_y_optical));
Serial.print('\n');

  delay(250);

  #endif
}
