/*#include <Arduino.h>
#include <Adafruit_SPIDevice.h>
#define LED 2
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>  //  Allows one to use ESP32's Wi-Fi functionalities
#include <MFRC522.h>
#include <string.h>

#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5
#define RST_PIN 4
#define SS_PIN 2

#define SSID "UserID"
#define PASSWORD "PASSWORD"

MFRC522 mfrc522(SS_PIN, RST_PIN);

const int buttonPin1 = 34;
int buttonState, B1_state;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 40;

//----------------------------------------------------------------------------------
//  Setting a static IP address
//    The following code assigns IP 192, 168, 1, 184 that works with gateway
//    192, 168, 1, 1

//  Set IP
IPAddress local_IP(192, 168, 1, 184);
//  Set gatway IP 
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //  Optional
IPAddress secondaryDNS(8, 8, 4, 4);  //  Optional
//----------------------------------------------------------------------------------
void setup()
{
  // Sensor Initialization
    SPI.begin();  // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
  //  Rest
    Serial.begin(115200);
    pinMode(LED, OUTPUT); 
    Serial.println("Hello");
  //  Configuring static IP
    if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA Failed to configure");
    }
}

void loop()
{
    Serial.println("Entered loop");
    if(mfrc522.PICC_IsNewCardPresent())//  RFID read here
    {
        Serial.println("Card Present");
        if(mfrc522.PICC_ReadCardSerial())
        {
            Serial.println("Card Read OK");
            String idcard = "";
            //Compute ID card
                for(byte i = 0; i < mfrc522.uid.size; i++)
                {
                    idcard += (mfrc522.uid.uidByte[i]< 0x10 ? "0" : "") + 
                    String(mfrc522.uid.uidByte[i], HEX);  //  Log ID card number
                }
            Serial.println("tag rfid :" + idcard);
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
        }
    }

    //  Reconnect to WiFi network after losing connection
        unsigned long currentMillis = millis();
        unsigned long previousMillis = 0;
        unsigned long interval = 30000;
    //  If WiFi is down, try reconnecting again
        if((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
        {
            //Serial.print(millis());
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect();    
            WiFi.reconnect();     
        }
}

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi ..");
    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}
*/
#/*#include <Arduino.h>
#include <WiFiMulti.h>

#define LED 2
#define WIFI_SSID       "NOWTVII8F7"    //  Name of network
#define WIFI_PASSWORD   "smuqjz6jYsNW"     //  Password
#include <Arduino.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

void setup() {
  Serial.begin(921600);
  pinMode(LED, OUTPUT);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  Serial.println("Connected");
}

void loop() {
  digitalWrite(LED, WiFi.status() == WL_CONNECTED);
}*/

#include <Arduino.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <SPI.h>

#define LED 2
//#define WIFI_SSID       "iPhone"    //  Name of network
//#define WIFI_PASSWORD   "abcdefghe"     //  Password
#define WIFI_SSID       "NOWTVII8F7"    //  Name of network
#define WIFI_PASSWORD   "smuqjz6jYsNW"     //  Password

WiFiMulti wifiMulti;
//------------------------------------------DEPENDENCIES--------------------------------------------//
void connection(const char* ssid, const char* passcode)
{
    //  Add access point
    
    wifiMulti.addAP(ssid, passcode);

    while(wifiMulti.run() != WL_CONNECTED)
    {
        //  If can't connect, delay and retry
        delay(100);
    }
    Serial.println("Connected\n");
}
//--------------------------------------------------------------------------------------------------//

void setup()
{
    // Monitor speed
        Serial.begin(115200);   
    //  Control voltage that goes to output
        pinMode(LED, OUTPUT);
        
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
    bool connection_status = (bool)(WiFi.status() == WL_CONNECTED);
    Serial.println( "WIFI CONNECTION STATUS: " 
                    + String((connection_status) ? "ON" : "OFF") 
                    + "\n");

    Serial.println("Reconnecting");
    //  Reconnection
    while(!connection_status)
    {
        Serial.println("..");
        connection(WIFI_SSID, WIFI_PASSWORD);
        connection_status = (bool)(WiFi.status() == WL_CONNECTED);
    }
}