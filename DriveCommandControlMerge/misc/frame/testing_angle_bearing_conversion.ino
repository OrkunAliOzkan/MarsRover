// PIO includes
  #include <SPI.h>
  #include <Wire.h>
  #include <WiFi.h>
  #include <WiFiMulti.h>

/////////////////////////////////////////////////////////////////

//  Optical flow sensor parameters
//slave select/chip select (ESP32 pin number)
  #define PIN_SS                  5 
//output data from mouse sensor (ESP32 pin number)
  #define PIN_MISO                19 
//input data to mouse sensor (ESP32 pin number)
  #define PIN_MOSI                23 
//synchronous clock (ESP32 pin number)
  #define PIN_SCK                 18 
//reset (ESP32 pin number)
  #define PIN_MOUSECAM_RESET      32 ////changed from 35! 
//chip select (ESP32 pin number again)
  #define PIN_MOUSECAM_CS         5 
#define ADNS3080_PIXEL_SUM      0x06
//constant for x-axis pixel count 
  #define ADNS3080_PIXELS_X       30 
//constant for y-axis pixel count 
  #define ADNS3080_PIXELS_Y       30 
//  Quick access registers that store pixel data
  #define ADNS3080_PIXEL_BURST    0x40 
//  Quick access registers that store motion data
  #define ADNS3080_MOTION_BURST   0x50
//read and write
  #define ADNS3080_FRAME_CAPTURE  0x13 

  #define RADIUS 122
/////////////////////////////////////////////////////////////////

//  Name of network
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"

/////////////////////////////////////////////////////////////////

// Motor Controller Pin mappings
  #define PWMA 17
  #define PWMB 2
  #define AIN1 14
  #define AIN2 16
  #define BIN1 4
  #define BIN2 15

/////////////////////////////////////////////////////////////////

// Starting coordinates
  float A_x = 0; // TODO: Define at end of movement
  float A_y = 0; // TODO: Define at end of movement

// Destination coordinates
  float B_x = 0;
  float B_y = 750;

// Current position and bearing
  float current_x = 0;
  float current_y = 0;
  float current_bearing = PI/2;

  float prev_x = 0;
  float prev_y = 0;
  float prev_bearing = PI/2;

/////////////////////////////////////////////////////////////////

//  min max PWM
  #define MIN_PWM 38
  #define MAX_PWM 220

//  Drive parameters
  int MotorSpeedA = 0; //  Final input to motors
  int MotorSpeedB = 0; //  Final input to motors

/////////////////////////////////////////////////////////////////

//  Variables for OFS_Cartesian()
  int prescaled_tx = 0;
  int prescaled_ty = 0;

  int totalpath_x_int = 0; // function output - total path in x component of OFS
  int totalpath_y_int = 0; // function output - total path in y component of OFS

/////////////////////////////////////////////////////////////////

// Variables for OFS_Angular
  float abs_theta = 0;
  float totalpath_x_flt = 0; // postscaling x displacement
  float totalpath_y_flt = 0; // postscaling y displacement

/////////////////////////////////////////////////////////////////

//  Angle Control: rotation
  float angular_error = 0;
  float angular_error_prev = 0;
  float target_angle = 0;

  float p_term_angle;
  float i_term_angle;
  float d_term_angle;

  bool Rot_Ctrl = 0;//  Informed by server

/////////////////////////////////////////////////////////////////

//  PID for turning
  float Kp_rotation = 2;
  float Ki_rotation = 0.02;
  float Kd_rotation = 0.2;

/////////////////////////////////////////////////////////////////

// PID for straight line
  float Kp_deviation = 3;
  float Ki_deviation = 0.05;
  float Kd_deviation = 0.05;
//  PWM output
  int differential_PWM_output;

  struct MD
  {
      byte motion;
      char dx, dy;
      byte squal;
      word shutter;
      byte max_pix;
  };

/////////////////////////////////////////////////////////////////
//  OFS Functions and Variables

  MD md;

int convTwosComp(int b)
{
    //Convert from 2's complement
    if(b & 0x80)
    {
      b = -1 * ((b ^ 0xff) + 1);
    }
    return b;
}

void mousecam_reset() //reset procedure = set reset high, followed by set reset low
{
    digitalWrite(PIN_MOUSECAM_RESET,HIGH);
    delay(1); // reset pulse >10us (constraint given in data sheet)
    digitalWrite(PIN_MOUSECAM_RESET,LOW);
    delay(35); // 35ms from reset to functional
}

int mousecam_init() //initialisation procedure
{
    pinMode(PIN_MOUSECAM_RESET,OUTPUT);
    pinMode(PIN_MOUSECAM_CS,OUTPUT);
    digitalWrite(PIN_MOUSECAM_CS,HIGH);
    mousecam_reset();
    return 1;
}

void mousecam_write_reg(int *reg, int *val) //write to mouse sensor's register 
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(*reg | 0x80); //address of register to write to sensor's register | buffer size?
    SPI.transfer(*val); //data to write to sensor's register
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(50); //required wait time
}

void mousecam_read_motion(struct MD *p)
{
    digitalWrite(PIN_MOUSECAM_CS, LOW); //activate serial port
    SPI.transfer(ADNS3080_MOTION_BURST); //reading from motion burst register == activation of Motion Read mode
    delayMicroseconds(75);
    p->motion =  SPI.transfer(0xff); //the sensor responds with the structure MD's data, in the given order
    p->dx =  SPI.transfer(0xff);
    p->dy =  SPI.transfer(0xff);
    p->squal =  SPI.transfer(0xff);
    p->shutter =  SPI.transfer(0xff)<<8;
    p->shutter |=  SPI.transfer(0xff);
    p->max_pix =  SPI.transfer(0xff);
    digitalWrite(PIN_MOUSECAM_CS,HIGH); //deactivate serial port
    delayMicroseconds(5); //necessary wait time
}

void OFS_Cartesian
            (            
            MD md, 
            int * prescaled_tx, 
            int * prescaled_ty, 
            int * total_x, 
            int * total_y
            )
{
  //  Optical sensor readings
    mousecam_read_motion(&md);

  *prescaled_tx += convTwosComp(md.dx);
  *prescaled_ty += convTwosComp(md.dy);

  *total_x = *prescaled_tx / 4.95;
  *total_y = *prescaled_ty / 4.95;

}

void OFS_Angular(
                MD md, 
                float * total_x, 
                float * total_y, 
                float* abs_theta
                )
{
      //  Optical sensor readings
        mousecam_read_motion(&md);
        *abs_theta += (convTwosComp(md.dx) / 4.95) / RADIUS;
        float d_r = convTwosComp(md.dy) / 4.95;
        *total_x += d_r * sin(*abs_theta);
        *total_y += d_r * cos(*abs_theta);
}

//  update variables
  long currT = 0;
  long prevT = 0;
  float deltaT = 0;

/////////////////////////////////////////////////////////////////

// PID for displacement

  float Kp_displacement = 3;
  float Ki_displacement = 0;
  float Kd_displacement = 0.05;

  int displacement_PWM_output = 128;  //  Displacement controllers PWM output
  int displacement_error = 0;
  int target_displacement = 0;


/////////////////////////////////////////////////////////////////

//  tcp related variables
String tcp_received = "";
String tcp_send = "";
String mode_ = "";

int tcp_parse(String tcp_data, float * B_x, float * B_y, String * mode_)
{
  String tmp = tcp_data;
  /*
  data = "x,y,mode"
  */
  *B_x = (tcp_data.substring(0, tcp_data.indexOf(","))).toFloat();
  tmp = tcp_data.substring(tcp_data.indexOf(",") + 1);
  *B_y = (tmp.substring(0, tmp.indexOf(","))).toFloat();
  *mode_ = tmp.substring(tmp.indexOf(",") + 1);

  return 1;
}

/////////////////////////////////////////////////////////////////

//  Wifi Initialisation
WiFiMulti WiFiMulti;
WiFiClient client; // Use WiFiClient class to create TCP connections
const uint16_t port = 8080;
const char * host = "146.169.171.197"; // ip or dns

// WiFi timeout variables (reliability)
  long wifi_last_connect_attempt = 0;
  long wifi_connect_timeout = 3000;
// tcp timeout variables (reliability)
  long tcp_last_connect_attempt = 0;
  long tcp_connect_timeout = 3000;
  
  long last_TCP_post = 0;
  long TCP_post_period = 50;

/////////////////////////////////////////////////////////////////

void updateTargets(float * B_x, float * B_y, float * current_x, float * current_y, float * current_bearing, int * target_displacement, float * target_angle){
    
    float dx = *B_x-*current_x;
    float dy = *B_y-*current_y;
    *target_angle = atan2( dx, dy ) - *current_bearing;
    //  converts from angle to bearing
      
    if((*target_angle) > PI){
        *target_angle -= PI;
        *target_angle *= -1;
    }
    else if ((*target_angle) < -PI){
        *target_angle += PI;
        *target_angle *= -1;
    }

    *target_angle *= -1;
    // update target displacement
    *target_displacement = (int) sqrt(pow(dy, 2) + pow(dx, 2));
}

//  state mashine parameters for the drive process (stop turn go)
  int turning_complete = 1;
  int straight_line_complete = 1;

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    /////////////////////////////////////////////////////////////////
    // Setting pinouts for motors
    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    /////////////////////////////////////////////////////////////////
    // Pinouts for OFS
    pinMode(PIN_SS,OUTPUT); //(CHIP SELECT)
    pinMode(PIN_MISO,INPUT); //(MASTER IN, SLAVE OUT)
    pinMode(PIN_MOSI,OUTPUT); //(MASTER OUT, SLAVE IN)
    pinMode(PIN_SCK,OUTPUT); //(CLOCK)
    /////////////////////////////////////////////////////////////////
    //  SPI Initialisation
    SPI.begin();
    //sets SPI clock to 1/32 of the ESP32's clock
    SPI.setClockDivider(SPI_CLOCK_DIV32); 
    //in SPI mode 3, data is sampled on the 
    //falling edge and shifted out on the rising edge
    SPI.setDataMode(SPI_MODE3); 
    SPI.setBitOrder(MSBFIRST);

    if(mousecam_init()==-1)
    {
        //Serial.println("Mouse cam failed to init");
        while(1);
    }
    /////////////////////////////////////////////////////////////////
    //  Connecting to Wifi
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\n");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);
    /////////////////////////////////////////////////////////////////
    //  Connecting to TCP Server
    if (!client.connect(host, port)) {
        Serial.println("Connection to TCP Server failed");
        Serial.println("Trying again in 500ms...");
        delay(500);
    }
    Serial.println("Connected to Server\n");

    // // wait for mission start
    String start_message = "";
    while(client.available() < 5){
        Serial.println("Waiting for Mission Start");
        delay(500);
    }


    tcp_received = client.readStringUntil('\r');
    tcp_parse(tcp_received, &B_x, &B_y, &mode_);

    // update target angle
    updateTargets(&B_x, &B_y, &current_x, &current_y, &current_bearing, &target_displacement, &target_angle);

    Serial.println("cx: " + String(current_x));
    Serial.println("cy: " + String(current_y));
    Serial.println("bx: " + String(B_x));
    Serial.println("by: " + String(B_y));
    Serial.println("current bearing: " + String(current_bearing));   
    Serial.println("target displacement: " + String(target_displacement));
    Serial.println("target angle: " + String(target_angle));
    turning_complete = 0;
    straight_line_complete = 0;
}

void loop()
{
    // Periodically send data back to server
    if (millis() - last_TCP_post > TCP_post_period) {
        Serial.println(tcp_send);
        // client.print(tcp_send);
        tcp_send = "";
        last_TCP_post = millis();
    }
    // checks if there is a message from server in buffer
    if (client.available() > 5) {
        // brake
        analogWrite(PWMA, 0); 
        analogWrite(PWMB, 0);

        //read back one line from the server
        // Serial.println("reading buffer");
        tcp_received = client.readStringUntil('\r');
        // Serial.println(line);

        //  parse data recieved
        tcp_parse(tcp_received, &B_x, &B_y, &mode_);

        // update target angle
        updateTargets(&B_x, &B_y, &current_x, &current_y, &current_bearing, &target_displacement, &target_angle);

        Serial.println("//////////////////////////////////////////////////////////////////");
        Serial.println("target displacement: " + String(target_displacement));
        Serial.println("target angle: " + String(target_angle));

        delay(5000);
        turning_complete = 0;
        straight_line_complete = 0;
    }

    if (!turning_complete) {
        // Rotation Logic
        totalpath_x_int = RADIUS*target_angle;
        angular_error = (totalpath_x_int - RADIUS*target_angle);
        // simplistic dead reckoning

        if (abs(angular_error) < 3) {
            turning_complete = 1;
            // brake
            prev_bearing -= target_angle;

            tcp_send = "---\n";
            Serial.println ("Turning Complete\n");
        } 

        // Debugging Messages
        Serial.println("---\n");
        Serial.println("Rover is turning");
        Serial.println("target_angle: " + String(target_angle));
    } 
    else if (!straight_line_complete) {
        // Straight Line Logic
        displacement_error = 0;
        
        if (abs(displacement_error) < 10) {

            straight_line_complete = 1;
            differential_PWM_output = 0; 

            // simplistic dead reckoning
            prev_x = B_x;
            prev_y = B_y;

            //  new
            current_x = B_x;
            current_y = B_y;
            
            // resetting PID variables
            prevT = 0;
            p_term_angle = 0;
            i_term_angle = 0;
            d_term_angle = 0;

            // resetting OFS_Cartesian variables
            prescaled_tx = 0;
            prescaled_ty = 0;
            totalpath_x_int = 0;
            totalpath_y_int = 0;

            tcp_send = "---\n";
            Serial.println ("Straight Line Complete\n");
        }

    //  debug content
        Serial.println("Rover moving in straight line:");
        Serial.println("current_x: " + String(current_x));
        Serial.println("current_y: " + String(current_y));
    }
}