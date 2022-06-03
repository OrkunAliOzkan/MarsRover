#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>

//------------------------------------------DEPENDENCIES--------------------------------------------//
#define LED 2
//#define WIFI_SSID       "iPhone"    //  Name of network
//#define WIFI_PASSWORD   "abcdefghe"     //  Password
#define WIFI_SSID       "NOWTVII8F7"    //  Name of network
#define WIFI_PASSWORD   "smuqjz6jYsNW"     //  Password
//--------------------------------------------------------------------------------------------------//
void setup()
{
  Serial.begin(921600);
  pinMode(LED, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Initializing");
}
/*  
    This code is a state mashine
    connecting when not and doing nothing when connected. 
    This indicator is used to determine whether it is so
*/
bool isConnected = false;

void loop() {
//  If desired to connect and can, express as connected
    if (WiFi.status() == WL_CONNECTED && !isConnected) {
        Serial.println("Connected");
        digitalWrite(LED, HIGH);
        isConnected = true;
    }

//  If not connected, connect and express as not connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(".");
        digitalWrite(LED, !digitalRead(LED));
        delay(1000);
        isConnected = false;
    }
}