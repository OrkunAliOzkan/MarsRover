#include "Arduino.h"
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "code_body.h"

#define PWMA    17
#define PWMB    2
#define AI1     14
#define AI2     16
#define BI1     4
#define BI2     15
#define STNDBY  4
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

        //  Determine substring that contains char
        //Serial.println(payload);
        //Serial.println("a");
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

void fclass::arm()
{
digitalWrite(STNDBY, HIGH);
}

void fclass::disarm()
{
digitalWrite(STNDBY, LOW);
}

void fclass::RightCW(int Speed)
{
int DutyRef = map(Speed,0,100,0,256);
digitalWrite(AI1, LOW);
digitalWrite(AI2, HIGH);
analogWrite(PWMA, DutyRef);
}

void fclass::RightCCW(int Speed)
{
int DutyRef = map(Speed,0,100,0,256);
digitalWrite(AI1,HIGH);
digitalWrite(AI2,LOW);
analogWrite(PWMA, DutyRef);
}

void fclass::LeftCW(int Speed)
{
int DutyRef = map(Speed,0,100,0,256);
digitalWrite(BI1,HIGH);
digitalWrite(BI1,LOW);
analogWrite(PWMB, DutyRef);
}

void fclass::LeftCCW(int Speed)
{
int DutyRef = map(Speed,0,100,0,256);
digitalWrite(BI1,HIGH);
digitalWrite(BI2,LOW);
analogWrite(PWMB, DutyRef);
}

void fclass::RightStop()
{
digitalWrite(AI1, LOW);
digitalWrite(AI2, LOW);
}

void fclass::LeftStop()
{
digitalWrite(BI1, LOW);
digitalWrite(BI2, LOW);
}

void fclass::RightTurn_Spot(unsigned long duration,int Speed)
{
RightCCW(Speed);
LeftCW(Speed);
delay(duration);
RightStop();
LeftStop();
}

void fclass::Forward(int duration,int Speed){
LeftCCW(Speed);
RightCW(Speed);
delay(duration);
RightStop();
LeftStop();
}

void fclass::LeftTurn_Spot(unsigned long duration,int Speed)
{
LeftCW(Speed);
RightCW(Speed);
delay(duration);
RightStop();
LeftStop();
}

void fclass::Backward(unsigned long duration,int Speed)
{
LeftCCW(Speed);
RightCCW(Speed);
delay(duration);
RightStop();
LeftStop();
}

fclass code_body = fclass();