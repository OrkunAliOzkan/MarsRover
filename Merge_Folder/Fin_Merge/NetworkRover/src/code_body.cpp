#include "Arduino.h"
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "code_body.h"

#define PWMA    17
#define PWMB    2
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
        String character = (doc[F("key")]);
        soln = character;
        Serial.println(payload);
        Serial.println(character);
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

void fclass::RCW(int DutyCycle)
{
int Duty_8 = map(DutyCycle,0,100,0,256);
digitalWrite(BIN1, HIGH);
digitalWrite(BIN2, LOW);
analogWrite(PWMB, Duty_8);
}

void fclass::RCCW(int DutyCycle)
{
int Duty_8 = map(DutyCycle,0,100,0,256);
digitalWrite(BIN1,LOW);
digitalWrite(BIN2,HIGH);
analogWrite(PWMB, Duty_8);
}

void fclass::LCW(int DutyCycle)
{
int Duty_8 = map(DutyCycle,0,100,0,256);
digitalWrite(AIN1,HIGH);
digitalWrite(AIN2,LOW);
analogWrite(PWMA, Duty_8);
}

void fclass::LCCW(int DutyCycle)
{
int Duty_8 = map(DutyCycle,0,100,0,256);
digitalWrite(AIN1,LOW);
digitalWrite(AIN2,HIGH);
analogWrite(PWMA, Duty_8);
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

void fclass::Forward(int Duration,int DutyCycle)
{
LCCW(DutyCycle);
RCW(DutyCycle);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::Backward(int Duration,int DutyCycle)
{
LCW(DutyCycle);
RCCW(DutyCycle);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::SpinCW(int Duration,int DutyCycle)
{
RCCW(DutyCycle);
LCCW(DutyCycle);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::SpinCCW(int Duration,int DutyCycle)
{
LCW(DutyCycle);
RCW(DutyCycle);
delay(Duration);
LSTOP();
RSTOP();
}

void fclass::ForwardL(int Duration,int DutyCycle)
{
    RCW(DutyCycle);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::ForwardR(int Duration,int DutyCycle)
{
    LCCW(DutyCycle);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::BackwardL(int Duration,int DutyCycle)
{
    RCCW(DutyCycle);
    delay(Duration);
    LSTOP();
    RSTOP();
}

void fclass::BackwardR(int Duration,int DutyCycle)
{
    LCW(DutyCycle);
    delay(Duration);
    LSTOP();
    RSTOP();
}

fclass code_body = fclass();