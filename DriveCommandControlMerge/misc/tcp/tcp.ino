  #include <WiFi.h>
  #include <WiFiMulti.h>
  WiFiMulti WiFiMulti;

  float timer;
  String post_msg = "";
  void setup()
  {
      Serial.begin(115200);
      delay(10);
      // We start by connecting to a WiFi network
      WiFiMulti.addAP("bet-hotspot", "helloworld");
      Serial.println();
      Serial.println();
      Serial.print("Waiting for WiFi... ");
      while(WiFiMulti.run() != WL_CONNECTED) {
          Serial.print(".");
          delay(500);
      }
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      delay(500);
      String lines = "---------------------------------------------------------------------";
  }
  void loop()
  {
      const uint16_t port = 3002;
      const char * host = "192.168.137.1"; // ip or dns
      // Serial.print("Connecting to ");
      // Serial.println(host);
      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      if (!client.connect(host, port)) {
          Serial.println("Connection failed.");
          Serial.println("Waiting 5 seconds before retrying...");
          delay(500);
          return;
      }
      // This will send a request to the server
      //uncomment this line to send an arbitrary string to the server
      client.print("Send this data to the server");
  //
  //  //wait for the server's reply to become available

    timer = millis();
    if(client.findLastOf('\r'))
    {
      if (client.available())
      {
        //read back one line from the server
        String line = client.readStringUntil('\r');
        Serial.println(timer);
      }
      else
      {
        Serial.println("client.available() timed out ");
      }
    }
      //Serial.println("Closing connection.");
      client.stop();
      //Serial.println("Waiting 5 seconds before restarting...");
      delay(500);
}