#ifndef ofs_h
#define ofs_h

#include <Arduino.h>
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
// Drive Pins
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15

class OFSClass
{
public:
    struct MD;
    int convTwosComp(int b);
    void mousecam_reset();
    int mousecam_init();
    void mousecam_write_reg(int *reg, int *val);
    void mousecam_read_motion(struct MD *p);
    double OFS_Squal(MD md);
}
#endif