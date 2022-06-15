// #include "Arduino.h"
// #include <SPI.h>
// #include <string>
// #include <vector>
// #include "code_body.h"
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #define PWMA 17
// #define PWMB 2
// #define AIN1 14
// #define AIN2 16
// #define BIN1 4
// #define BIN2 15
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #define RADIUS 122
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #define PIN_SS                            5 
// #define PIN_MISO                          19 
// #define PIN_MOSI                          23 
// #define PIN_SCK                           18 
// #define PIN_MOUSECAM_RESET                32 
// #define PIN_MOUSECAM_CS                   5 
// #define ADNS3080_PIXEL_SUM                0x06
// #define ADNS3080_PIXELS_X                 30 
// #define ADNS3080_PIXELS_Y                 30 
// #define ADNS3080_PIXEL_BURST              0x40 
// #define ADNS3080_MOTION_BURST             0x50
// #define ADNS3080_FRAME_CAPTURE            0x13 
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #define DECLINATIONANGLE 0.483 /* * (PI / 180)   FIXME: Isn't this in rad?*/
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fclass::fclass()
// {}

// String URL = "http://146.169.170.159:3001/";
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void fclass::OFS_Cartesian
//             (            
//             MD md, 
//             int * prescaled_tx, 
//             int * prescaled_ty, 
//             int * total_x, 
//             int * total_y
//             )
// {
//   //  Optical sensor readings
//     code_body.mousecam_read_motion(&md);

//   *prescaled_tx += code_body.convTwosComp(md.dx);
//   *prescaled_ty += code_body.convTwosComp(md.dy);

//   *total_x = *prescaled_tx / 4.95;
//   *total_y = *prescaled_ty / 4.95;

//   //Serial.println("Dy: " + String(convTwosComp(md.dy)));
//   //Serial.println("Dx: " + String(convTwosComp(md.dx)));
//   //Serial.println("Total y: " + String(*total_y));
//   //Serial.println("Total x: " + String(*total_x));
//   //Serial.println();
// }

// void fclass::OFS_Angular(
//                 MD md, 
//                 float * total_x, 
//                 float * total_y, 
//                 float* abs_theta
//                 )
// {
//       //  Optical sensor readings
//         code_body.mousecam_read_motion(&md);
//         *abs_theta += (code_body.convTwosComp(md.dx) / 4.95) / RADIUS ;
//         float d_r = code_body.convTwosComp(md.dy) / 4.95;
//         *total_x += d_r * sin(*abs_theta);
//         *total_y += d_r * cos(*abs_theta);
//         //Serial.println("dr: " + String(d_r));
//         //Serial.println("d_theta: "+ String((code_body.convTwosComp(md.dx) / 4.95) / RADIUS));
//         //Serial.println("Angle: " + String(*abs_theta));
//         //Serial.println("Total y: " + String(*total_y));
//         //Serial.println("Total x: " + String(*total_x));
// }

// int fclass::convTwosComp(int b)
// {
//     //Convert from 2's complement
//     if(b & 0x80)
//     {
//       b = -1 * ((b ^ 0xff) + 1);
//     }
//     return b;
// }

// void fclass::mousecam_reset() //reset procedure = set reset high, followed by set reset low
// {
//     digitalWrite(PIN_MOUSECAM_RESET,HIGH);
//     delay(1); // reset pulse >10us (constraint given in data sheet)
//     digitalWrite(PIN_MOUSECAM_RESET,LOW);
//     delay(35); // 35ms from reset to functional
// }

// int fclass::mousecam_init() //initialisation procedure
// {
//     pinMode(PIN_MOUSECAM_RESET,OUTPUT);
//     pinMode(PIN_MOUSECAM_CS,OUTPUT);
//     digitalWrite(PIN_MOUSECAM_CS,HIGH);
//     mousecam_reset();
//     return 1;
// }

// void fclass::mousecam_write_reg(int *reg, int *val) //write to mouse sensor's register 
// {
//     digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
//     SPI.transfer(*reg | 0x80); //address of register to write to sensor's register | buffer size?
//     SPI.transfer(*val); //data to write to sensor's register
//     digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
//     delayMicroseconds(50); //required wait time
// }

// void fclass::mousecam_read_motion(struct MD *p)
// {
//     digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
//     SPI.transfer(ADNS3080_MOTION_BURST); //reading from motion burst register == activation of Motion Read mode
//     delayMicroseconds(75);
//     p->motion =  SPI.transfer(0xff); //the sensor responds with the structure MD's data, in the given order
//     p->dx =  SPI.transfer(0xff);
//     p->dy =  SPI.transfer(0xff);
//     p->squal =  SPI.transfer(0xff);
//     p->shutter =  SPI.transfer(0xff)<<8;
//     p->shutter |=  SPI.transfer(0xff);
//     p->max_pix =  SPI.transfer(0xff);
//     digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
//     delayMicroseconds(5); //necessary wait time
// }

// fclass code_body = fclass();