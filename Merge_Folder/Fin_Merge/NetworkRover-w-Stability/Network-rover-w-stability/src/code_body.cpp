#include "Arduino.h"
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "code_body.h"

#define PWMA    17  // left
#define PWMB    2   // right
#define AIN1    14
#define AIN2    16
#define BIN1    4
#define BIN2    15
#define STANDBY 32
DynamicJsonDocument doc(1024);
String URL = "http://54.242.65.190:8000/";

fclass::fclass()
{}

void fclass::test_code()
{
    Serial.println("hello\n");
}

String fclass::HTTPGET()
{
    String soln;
    HTTPClient http;
    http.begin(URL.c_str()); // URL
    int httpReturn = http.GET();

    if(httpReturn > 0)
    {
        String payload = http.getString();
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();
        String angle = (doc[F("ang")]);
        String magnitude = (doc[F("mag")]);
        String 
        soln = angle;
        soln += " ";
        soln += magnitude;
        Serial.println(payload);
        Serial.println(soln);
    }
    else
    {
        Serial.println("Error on HTTP request\n");
        soln = ".";
    }

    http.end();
    return soln;
}

/*void fclass::HTTPPOST()
{
    HTTPClient http;

    http.begin(URL); // URL
    http.addHeader("Content-Type", "text/plain");
    int httpDump = http.POST("SOS\n");

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
*/

void fclass::ARM()
{
digitalWrite(STANDBY, HIGH);
}

void fclass::DISARM()
{
digitalWrite(STANDBY, LOW);
}

void fclass::RCW(int DutyCycleRight)
{
int Duty_8_right = map(DutyCycleRight,0,100,0,256);
digitalWrite(BIN1, HIGH);
digitalWrite(BIN2, LOW);
analogWrite(PWMB, Duty_8_right);
}

void fclass::RCCW(int DutyCycleRight)
{
int Duty_8_right = map(DutyCycleRight,0,100,0,256);
digitalWrite(BIN1,LOW);
digitalWrite(BIN2,HIGH);
analogWrite(PWMB, Duty_8_right);
}

void fclass::LCW(int DutyCycleLeft)
{
int Duty_8_left = map(DutyCycleLeft,0,100,0,256);
digitalWrite(AIN1,HIGH);
digitalWrite(AIN2,LOW);
analogWrite(PWMA, Duty_8_left);
}

void fclass::LCCW(int DutyCycleLeft)
{
int Duty_8_left = map(DutyCycleLeft,0,100,0,256);
digitalWrite(AIN1,LOW);
digitalWrite(AIN2,HIGH);
analogWrite(PWMA, Duty_8_left);
}

void fclass::RSTOP()
{
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, LOW);
}

void fclass::LSTOP()
{
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, LOW);
}

void fclass::Forward(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
LCCW(DutyCycleLeft);
RCW(DutyCycleRight);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::Backward(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
LCW(DutyCycleLeft);
RCCW(DutyCycleRight);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::SpinCW(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
RCCW(DutyCycleRight);
LCCW(DutyCycleLeft);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::SpinCCW(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
LCW(DutyCycleLeft);
RCW(DutyCycleRight);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::ForwardL(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
    RCW(DutyCycleRight);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::ForwardR(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
    LCCW(DutyCycleLeft);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::BackwardL(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
    RCCW(DutyCycleRight);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::BackwardR(int Duration,int DutyCycleLeft, int DutyCycleRight)
{
    LCW(DutyCycleLeft);
    delay(Duration);
    LSTOP();
    RSTOP();
}

fclass code_body = fclass();