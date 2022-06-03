#include <Arduino.h>
#include <Adafruit_SPIDevice.h>
#define LED 2
/*
void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);  
}

void loop() {
  //  this is a function which controls output from ARDUINO pins.
  digitalWrite(LED, HIGH);  
  Serial.println("LED is on!");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("LED is off!");
  delay(1000);
}
*/

//Debounce a pushdown:
/*
    When button is pressed/released or when a switch is stoggled between ON or OFF, 
    its state is changed from LOW to HIGH or HIGH to LOW once. Is this correct? 
    Pushbuttons often generate spurious open/close transactions when pressed, 
    due to mechancial and physcal issues: these transitions may be read as 
    multple [resses ina  short time frame, fooling the program.

    const int buttonPin1 = 34;
    int buttonState, B1_state;
    int lastButtonState = LOW;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 40;


  void setup() {
    Serial.begin(115200);
    pinMode(LED, OUTPUT);  
  }

  int read_state_button(int buttonPin)
  {
    buttonState = 0;
    int reading = digitalRead(buttonPin); //  Reading inputs
    if(reading != lastButtonState)        //  If new input
    {
      lastDebounceTime = millis();        //  Log last debounce time
    }
    delay(debounceDelay + 1);
    //  If time between last debounce time and log is greater than the deboune delay,
    //  and the reading character is not the buttonstate:
    if(((millis() - lastDebounceTime) > debounceDelay))
    {
      if(reading != buttonState)
      {
        buttonState = reading;    //  Update button state
      }
    }
    lastButtonState = reading;  
    return buttonState;
  }

  void loop()
  {
    B1_state = read_state_button(buttonPin1);
    if(B1_state = HIGH)
    {
        Serial.println("Button pushed!");
    }
  }
*/

//SPI Communication
/*
  Shift Peripheral Interface communication
WiFi.mode(WIFI_STA): station mode: esp32 connects to access point
WiFi.mode(WIFI_AP):  access point mode: stations can connect to ESP32
WiFi.mode(WIFI_STA_AP): access point and a station 
                        connect to another access point

-----------------------------------------------------------------------------------
*/
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

int read_state_button(int buttonPin)
{
  buttonState = 0;
  int reading = digitalRead(buttonPin); //  Reading inputs
  if(reading != lastButtonState)        //  If new input
  {
    lastDebounceTime = millis();        //  Log last debounce time
  }
  delay(debounceDelay + 1);
  // If time between last debounce time and log is greater than the deboune delay,
  //and the reading character is not the buttonstate:
  if((millis() - lastDebounceTime) > debounceDelay)
  {
    if(reading != buttonState)
    {
      buttonState = reading;    //  Update button state

    }
  }
  lastButtonState = reading;  
  return buttonState;
}

void loop()
{
  // B1_state = read_state_button(buttonPin1);
  // if(B1_state = HIGH)
  // {
  //     //Serial.println("Button pushed!");
  // }
  Serial.println("Entered loop");
  if(mfrc522.PICC_IsNewCardPresent())//  RFID read here
  {
    Serial.println("Card Present");
    if(mfrc522.PICC_ReadCardSerial())
    {
      Serial.println("Card Read OK");
      String idcard = "";
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
      Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();    
      WiFi.reconnect();     
    }
}

void initWiFi()
{
  const char* ssid = "esp32userID";
  const char* password = "12345";
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

//Connection to database
//  Build an ESP32 client that sends HTTP POST request 
//  to a PHP script to insert data into a MySQL database.
/*

*/