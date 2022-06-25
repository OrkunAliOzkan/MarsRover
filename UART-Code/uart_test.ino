#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <SoftwareSerial.h>

//  Name of network
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"

//  Wifi Initialisation
WiFiClient client; // Use WiFiClient class to create TCP connections
const uint16_t port = 8080;
const char * host = "146.169.168.102"; // ip or dns

#define RXD2 16
#define TXD2 17

String hex_map(int x){
    if (x < 10) {
        return String(x);
    } else {
        switch (x) {
            case 10:
                return "A";
            case 11:
                return "B";
            case 12:
                return "C";
            case 13:
                return "D";
            case 14:
                return "E";
            case 15:
                return "F";
            default:
                return "0";
        }
    }
}

String byte_to_hex(int x){
    String tmp = hex_map(x & 0x0F);
    x >>= 2;
    return tmp += hex_map(x & 0x0F);
}

SoftwareSerial fpgaUART = SoftwareSerial(RXD2, TXD2, false);

void setup()
{
    // Software Serial
    pinMode(RXD2, INPUT);
    pinMode(TXD2, OUTPUT);
    fpgaUART.begin(115200);

    Serial.begin(115200);
    // Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    Wire.begin();
    /////////////////////////////////////////////////////////////////
    //  SPI Initialisation
    SPI.begin();
    //sets SPI clock to 1/32 of the ESP32's clock
    SPI.setClockDivider(SPI_CLOCK_DIV32); 
    //in SPI mode 3, data is sampled on the 
    //falling edge and shifted out on the rising edge
    SPI.setDataMode(SPI_MODE3); 
    SPI.setBitOrder(MSBFIRST);
    pinMode(LED_BUILTIN, OUTPUT);
    //  Connecting to Wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
   Serial.println("\n");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
//     /////////////////////////////////////////////////////////////////
//      //  Connecting to TCP Server
//      while (!client.connect(host, port)) {
// //         Serial.println("Connection to TCP Server failed");
// //         Serial.println("Trying again in 500ms...");
//          delay(500);
//      }
//      //Serial.println("Connected to Server\n");
//      client.print("hello there \n");
}
String incomingStr = "";

void loop() {
  int incomingByte = 0; // for incoming serial data
//  client.print("hello in loop \n");
//  if(Serial2){
//  }
//  incomingByte  = Serial2.read();
//  Serial.println(incomingByte);
    // if (Serial2.available() >= 4) {
    //     for (int i = 0; i < 4; i++) {
    //         incomingByte = Serial2.read();
    //         char conv = incomingByte;
    //         // Serial.print(byte_to_hex(incomingByte));
    //         Serial.print(incomingByte);
    //     }
    //     Serial.println();
    // }
    // while (Serial2.available() > 0) {
    //     incomingByte = Serial2.read();
    //     char conv = incomingByte;
    //     // Serial.print(byte_to_hex(incomingByte));
    //     Serial.println(byte_to_hex(incomingByte));
    // }
    if (fpgaUART.availabe() > 0) {
        int fpgaOut = fpgaUART.read();
        Serial.println(fpgaOut, HEX);
    }
}