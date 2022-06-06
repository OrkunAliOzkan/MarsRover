#include <Arduino.h>
#include "WiFi.h"
#include <SPI.h>
#include "code_body.h"
//-----------DEPENDENCIES-----------------
#define LED 2
//  Name of network
//    #define WIFI_SSID       "iPhone"    
//  Password
//    #define WIFI_PASSWORD   "abcdefghe"     
//  Name of network
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"     
/*  
    This code is a state mashine
    connecting when not and doing 
    nothing when connected. 
    This indicator is used to 
    determine whether it is so
*/
bool isConnected = false;
//----------------------------------------
void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    Serial.println("Test");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Initializing");
}

void loop() {
//  If desired to connect 
//  and can, express as connected
    //Serial.println(WiFi.status());

    if (WiFi.status() == WL_CONNECTED) {
        //digitalWrite(LED, HIGH);
        isConnected = true;

        Serial.println("--------------------------");
        String content = code_body.HTTPGET();
        code_body.HTTPPOST(content.c_str());
    }

//  If not connected, connect 
//  and express as not connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
        //digitalWrite(LED, 
        //            !digitalRead(LED));
        delay(1000);
        isConnected = false;
    }
}