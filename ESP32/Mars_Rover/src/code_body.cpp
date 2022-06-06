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

String URL = "http://146.169.174.8:3001/rover_request";

String fclass::HTTPGET()
{
    HTTPClient http;

    String payload = "";
    http.begin(URL); // URL

    int httpReturn = http.GET();

    payload = http.getString();
    Serial.println(httpReturn);
    Serial.println(payload);

    http.end();
    return payload;
}

void fclass::HTTPPOST(const char receivedChars[1024])
{
    HTTPClient http;

    http.begin(URL); // URL
    http.addHeader("Content-Type", "application/json");
    int httpDump = http.POST(receivedChars);

    if(httpDump > 0)
    {
        String responce = http.getString();
        Serial.println(httpDump);
        //Serial.println(responce);
    }
    else
    {
        Serial.println("Error on sending POST\n");
    }

    http.end();
}

fclass code_body = fclass();