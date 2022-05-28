#include "Arduino.h"
#include <SPI.h>
#include <HTTPClient.h>
#include "code_body.h"

fclass::fclass()
{}

void fclass::test_code()
{
    Serial.println("hello\n");
}

void fclass::HTTPGET()
{
    HTTPClient http;

    http.begin("ec2-54-242-65-190.compute-1.amazonaws.com"); // URL
    int httpReturn = http.GET();

    if(httpReturn > 0)
    {
        String payload = http.getString();
        Serial.println(httpReturn);
        Serial.println(payload);
    }
    else
    {
        Serial.println("Error on HTTP request\n");
    }

    http.end();
}

void fclass::HTTPPOST()
{
    HTTPClient http;

    http.begin("ec2-54-242-65-190.compute-1.amazonaws.com"); // URL
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

fclass code_body = fclass();