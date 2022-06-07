#include "Arduino.h"
#include <SPI.h>
#include <string>
#include <vector>
#include <HTTPClient.h>
#include "QMC5883LCompass.h"
#include "code_body.h"

#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
#define XJOY 27
#define YJOY 26
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PIN_SS                            5 
#define PIN_MISO                          19 
#define PIN_MOSI                          23 
#define PIN_SCK                           18 
#define PIN_MOUSECAM_RESET                32 
#define PIN_MOUSECAM_CS                   5 
#define ADNS3080_PIXEL_SUM                0x06
#define ADNS3080_PIXELS_X                 30 
#define ADNS3080_PIXELS_Y                 30 
#define ADNS3080_PIXEL_BURST              0x40 
#define ADNS3080_MOTION_BURST             0x50
#define ADNS3080_FRAME_CAPTURE            0x13 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLINATIONANGLE 0.483 /* * (PI / 180)   FIXME: Isn't this in rad?*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int numChars = 1024;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int speedA = 0;
//int speedB = 0;

fclass::fclass()
{}

String URL = "http://54.152.50.122:3001/rover_request";

std::vector<float> fclass::HTTPGET()
{
    HTTPClient http;

    std::vector<float> soln = {0, 0};
    size_t index_x;
    size_t index_y;
    String x_content, y_content;

    http.begin(URL); // URL
    int httpReturn = http.GET();

    if(httpReturn > 0)
    {
        String payload = http.getString();
        Serial.println(httpReturn);
        Serial.println(payload);
        index_x = payload.indexOf("x");
        index_y = payload.indexOf("y");
        Serial.println(index_x);
        Serial.println(index_y);

        //Serial.println("-----------------------");
        x_content = payload.substring(
                                    index_x + 5, 
                                    index_y - 4);
        //Serial.println(x_content);
        y_content = payload.substring(
                                index_y + 5, 
                                payload.length()-2);
        //Serial.println(y_content);
        //Serial.println(x_content.toFloat());
        //Serial.println("-----------------------");
        soln[0] = x_content.toFloat();
        soln[1] = y_content.toFloat();
        //Serial.println("-----------------------");
    }
    else
    {
        Serial.println("Error on HTTP request\n");
        soln[0] = 0;// Both are impossible inputs
        soln[1] = 0;// Both are impossible inputs
    }

    http.end();
    return soln;
}

void fclass::HTTPPOST(String receivedChars)
{
       // an array to store the received data
    HTTPClient http;
    Serial.println(receivedChars);
    http.begin(URL); // URL
    http.addHeader("Content-Type", "text/plain");
    int httpDump = http.POST(receivedChars);

    if(httpDump > 0)
    {
        String responce = http.getString();
        Serial.println(httpDump);
        Serial.println(responce);
    }
    else
    {
        Serial.println("Error on sending POST\n");
    }

    http.end();
}


double fclass::computeAngle(int x, int y)
{
    int angle = atan2(y, x);
    angle += DECLINATIONANGLE;
    
    if(angle < 0) 
        angle += 2*PI; 
    if(angle > 2*PI)
        angle -= 2*PI;

Serial.println("angle from computeAngle (degrees) = " + String(angle * 180));
    return angle;

    
}

float fclass::vector_multiply(std::vector<float> x, std::vector<float> y)
{
    //  It is known that x is actually a transpose
    float soln;
    for(int i = 0; i < x.size(); i++)
    {
        soln += (x[i]*y[i]);
    }

    return soln;
}

void fclass::readings(          int counter_input, 
                                QMC5883LCompass compass, 
                                float *headingDegrees,
                                int *distance_x,
                                int *distance_y,
                                int *total_x1,
                                int *total_y1,
                                int *total_x,
                                int *total_y
                                )
{
    //if((counter_input % 2) == 0)
    {
    //  Compass readings
        compass.read(); 
        Serial.println("a");     
        Serial.println(compass.getAzimuth());     
        *headingDegrees = compass.getAzimuth();
        delay(300); //??
    }
    Serial.println("I LIVE BITCH");
    //if((counter_input % 2) != 0)
    {
    //  Optical sensor readings
        int val = mousecam_read_reg(ADNS3080_PIXEL_SUM);
        MD md;
        mousecam_read_motion(&md);
        delay(300);

        *distance_y = convTwosComp(md.dy);
        *distance_x = convTwosComp(md.dx);
        *total_x1 = *total_x1 + *distance_x;
        *total_y1 = *total_y1 + *distance_y;
        *total_x = *total_x1/4.95;
        Serial.println(*total_x); 
        *total_y = *total_y1/4.95;
        Serial.println(*total_y);
    }
}

int fclass::convTwosComp(int b)
{
    //Convert from 2's complement
    if(b & 0x80)
    {
      b = -1 * ((b ^ 0xff) + 1);
    }
    return b;
}

void fclass::mousecam_reset() //reset procedure = set reset high, followed by set reset low
{
    digitalWrite(PIN_MOUSECAM_RESET,HIGH);
    delay(1); // reset pulse >10us (constraint given in data sheet)
    digitalWrite(PIN_MOUSECAM_RESET,LOW);
    delay(35); // 35ms from reset to functional
}

int fclass::mousecam_init() //initialisation procedure
{
    pinMode(PIN_MOUSECAM_RESET,OUTPUT);
    pinMode(PIN_MOUSECAM_CS,OUTPUT);
    digitalWrite(PIN_MOUSECAM_CS,HIGH);
    mousecam_reset();
    return 1;
}

void fclass::mousecam_write_reg(int *reg, int *val) //write to mouse sensor's register 
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(*reg | 0x80); //address of register to write to sensor's register | buffer size?
    SPI.transfer(*val); //data to write to sensor's register
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(50); //required wait time
}

int fclass::mousecam_read_reg(int reg) //read from mouse sensor's register
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(reg); ///address of register to read from sensor's register
    delayMicroseconds(75);
    int ret = SPI.transfer(0xff);
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(1);
    return ret; //value read from register
}

void fclass::mousecam_read_motion(struct MD *p)
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

fclass code_body = fclass();