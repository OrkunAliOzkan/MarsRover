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

String fclass::HTTPGET()
{
    HTTPClient http;

    String payload = "";
    http.begin("http://172.31.16.189:8000/"); // URL
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
    return payload;
}

void fclass::HTTPPOST(const char receivedChars[1024])
{
    HTTPClient http;

    http.begin("http://172.31.16.189:8000/"); // URL
    http.addHeader("Content-Type", "application/json");
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

fclass code_body = fclass();