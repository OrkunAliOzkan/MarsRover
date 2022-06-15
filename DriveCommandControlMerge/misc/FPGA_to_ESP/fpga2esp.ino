#include <ESP32DMASPIMaster.h>

ESP32DMASPI::Master master;

static const uint32_t BUFFER_SIZE = 8192;
uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;

///////////////////////////////////////////////////////////////////////////////////////////
String buffer_content = "";
///////////////////////////////////////////////////////////////////////////////////////////

void set_buffer() {
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        spi_master_tx_buf[i] = i & 0xFF;
    }
    memset(spi_master_rx_buf, 0, BUFFER_SIZE);
}

void setup() {
    Serial.begin(115200);
    ///////////////////////////////////////////////////////////////////////////////////////
    // to use DMA buffer, use these methods to allocate buffer
    spi_master_tx_buf = master.allocDMABuffer(BUFFER_SIZE);
    spi_master_rx_buf = master.allocDMABuffer(BUFFER_SIZE);
    set_buffer();
    delay(5000);
    master.setDataMode(SPI_MODE0);           // default: SPI_MODE0
    master.setFrequency(4000000);            // default: 8MHz (too fast for bread board...)
    master.setMaxTransferSize(BUFFER_SIZE);  // default: 4092 bytes
    master.begin();  // default: HSPI (CS: 15, CLK: 14, MOSI: 13, MISO: 12)
    ///////////////////////////////////////////////////////////////////////////////////////
}

void loop() {
    ///////////////////////////////////////////////////////////////////////////////////////
    // start and wait to complete transaction
    master.transfer(spi_master_tx_buf, spi_master_rx_buf, BUFFER_SIZE);
    //  To obtain the content of the buffer. It is string.
    for (size_t i = 0; i < BUFFER_SIZE; ++i) 
    {
        buffer_content += spi_master_rx_buf[i];
    }
    //  to display the string
    Serial.println(buffer_content);
    Serial.println("\n");
    ///////////////////////////////////////////////////////////////////////////////////////
    delay(2000);
}
