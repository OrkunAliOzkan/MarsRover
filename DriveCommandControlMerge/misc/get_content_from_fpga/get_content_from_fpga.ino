#include <SPI.h>
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
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
}

        String Str = "";


void loop(){
        Str = Serial.read();
        Serial.println(Str);  
}
