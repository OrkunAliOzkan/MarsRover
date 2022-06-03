#include <Arduino.h>
#include <WiFiMulti.h>

#define LED 2
#define WIFI_SSID       "iPhone"    //  Name of network
#define WIFI_PASSWORD   "abcdefghe"     //  Password

WiFiMulti wifiMulti;

void setup()
{
    // Monitor speed
        Serial.begin(115200);   
    //  Control voltage that goes to output
        pinMode(LED, OUTPUT);
    //  Add access point
        wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    while(wifiMulti.run() != WL_CONNECTED)
    {
        //  If can't connect, delay and retry
        delay(100);
    }

    Serial.println("Connected\n");
}


void loop()
{
    //  Use LED to determine the wifi status
    digitalWrite(LED, WiFi.status() == WL_CONNECTED);
}
