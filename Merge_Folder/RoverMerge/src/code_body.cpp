#include "Arduino.h"
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "code_body.h"
DynamicJsonDocument doc(1024);
String URL = "http://54.242.65.190:8000/";
fclass::fclass()
{}

void fclass::test_code()
{
    Serial.println("hello\n");
}

void fclass::HTTPGET()
{
    HTTPClient http;

    http.begin(URL.c_str()); // URL
    int httpReturn = http.GET();

    if(httpReturn > 0)
    {
        String payload = http.getString();
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();
        String character = doc[F("key")];
        Serial.println(payload);
        Serial.println(character);
        //  Determine substring that contains char
        //Serial.println(payload);
        //Serial.println("a");
    }
    else
    {
        Serial.println("Error on HTTP request\n");
    }

    http.end();
}
/*Requirement for arduino is to get key*/
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
*/
//    http.end();
//}

fclass code_body = fclass();