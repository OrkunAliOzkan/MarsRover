#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>

#define RADIUS 132
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

/////////////////////////////////////////////////////////////////

// Motor Controller Pin mappings
  #define PWMA 17
  #define PWMB 2
  #define AIN1 14
  #define AIN2 16
  #define BIN1 4
  #define BIN2 15

/////////////////////////////////////////////////////////////////

// //  Name of network
//     #define WIFI_SSID       "ooo20"    
// //  Password
//     #define WIFI_PASSWORD   "naber_lan"
    //  Name of network
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"
/*  
  //  Name of network
  //    #define WIFI_SSID       "NOWTVII8F7"    
  //  Password
  //    #define WIFI_PASSWORD   "smuqjz6jYsNW"     
  //  Name of network
  //      #define WIFI_SSID       "SIVA_LAPTOP"    
  //  Password
  //      #define WIFI_PASSWORD   "sivashanth"
  //  Name of network
  //    #define WIFI_SSID       "Orkun's Laptop"    
  //  Password
  //    #define WIFI_PASSWORD   "484f17Ya"
  //  Name of network
  //    #define WIFI_SSID       "CommunityFibre10Gb_003D7"    
  //  Password
  //  #define WIFI_PASSWORD   "gxqxs3c3fs" 
*/

/////////////////////////////////////////////////////////////////

//  OFS Functions and Variables
struct MD
  {
      byte motion;
      char dx, dy;
      byte squal;
      word shutter;
      byte max_pix;
  };
  
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

double total_path_x = 0;
double total_path_y = 0;

void OFS_Readings
            (            
            MD md, 
            double * total_path_x, 
            double * total_path_y, 
            double * x_coordinate, 
            double * y_coordinate,
            double * angle
            )
{
  //  Optical sensor readings
    mousecam_read_motion(&md);

    double dx = convTwosComp(md.dx) / 4.95;
    double dy = convTwosComp(md.dy) / 4.95;

    // calculating total oath moved in each direction
    *total_path_x += dx;
    *total_path_y += dy;

    double tmp_angle = *angle + dx / RADIUS / 2;
    *angle += dx / RADIUS;

    // update absolute coordinates 
    *x_coordinate += dy * cos(tmp_angle);
    *y_coordinate += dy * sin(tmp_angle);

    Serial.println("Dy: " + String(dy));
    Serial.println("Dx: " + String(dx));
    Serial.println("Angle: " + String(*angle));
    Serial.println("Total path y: " + String(*total_path_y));
    Serial.println("Total path x: " + String(*total_path_x));
    Serial.println("y_coordinate: " + String(*y_coordinate));
    Serial.println("x_coordinate: " + String(*x_coordinate));
    Serial.println();
}
/////////////////////////////////////////////////////////////////

//  Wifi Initialisation

WiFiClient client; // Use WiFiClient class to create TCP connections
const uint16_t port = 8080;
const char * host = "146.169.168.102"; // ip or dns

// WiFi timeout variables (reliability)
  long wifi_last_connect_attempt = 0;
  long wifi_connect_timeout = 3000;
// tcp timeout variables (reliability)
  long tcp_last_connect_attempt = 0;
  long tcp_connect_timeout = 3000;
  
  long last_TCP_send = 0;
  long TCP_post_period = 100;

/////////////////////////////////////////////////////////////////

// TCP Parser
//  tcp related variables
String tcp_received = "";
String mode_ = "";
String tcp_send = "";

int tcp_parse(String tcp_data, double * B_x, double * B_y, String * mode_)
{
  String tmp = tcp_data;
  /*
  data = "x,y,mode"
  */
  *B_x = (tcp_data.substring(0, tcp_data.indexOf(","))).toDouble();
  tmp = tcp_data.substring(tcp_data.indexOf(",") + 1);
  *B_y = (tmp.substring(0, tmp.indexOf(","))).toDouble();
  *mode_ = tmp.substring(tmp.indexOf(",") + 1);

  return 1;
}

/////////////////////////////////////////////////////////////////

//  Control Loop
  long currT = 0;
  long prevT = 0;
  double deltaT = 0;

/////////////////////////////////////////////////////////////////

// Destination coordinates
  double B_x = 0;
  double B_y = 750;

// Current position and angle
  double current_x = 100;
  double current_y = 120;
  double current_angle = 0;

/////////////////////////////////////////////////////////////////

// Drive parameters
  #define MIN_ROTATION_PWM 100 
  #define MAX_ROTATION_PWM 220

  #define STRAIGHT_PWM 128

  int MotorSpeedA = 0; //  Final input to motors
  int MotorSpeedB = 0; //  Final input to motors

/////////////////////////////////////////////////////////////////

//  Angle Control: rotation
  double angular_error = 0;
  double angular_error_prev = 0;
  double target_angle = 0;

  double p_term_angle;
  double i_term_angle;
  double d_term_angle;

  double Kp_rotation = 4;
  double Ki_rotation = 0.02;
  double Kd_rotation = 0.2;

//  Angle Control: rotation deviation
  double rotation_deviation_error = 0;
  double p_term_rot_deviation;

  double Kp_rot_deviation = 4;

/////////////////////////////////////////////////////////////////

// PID for deviation (during straight line motion)

  double Kp_deviation = 1;
  double Ki_deviation = 0;
  double Kd_deviation = 1;

/////////////////////////////////////////////////////////////////

// PID for displacement

  double Kp_displacement = 3;
  double Ki_displacement = 0;
  double Kd_displacement = 0.05;

  int displacement_error = 0;
  
  //  PWM output
  int differential_PWM_output = 0;
  int target_displacement = 0;

/////////////////////////////////////////////////////////////////
//  obstacle avoidance

//  camera readings
void camera_readings(int *camera_readings_type, double *camera_readings_displacemet, double *camera_readings_angle) {
*camera_readings_type = 7;

//  object is at (30, 30)
*camera_readings_displacemet = 27;
*camera_readings_angle = -PI/2;
}

int sign(double number){
    return (number >= 0) ? (1) : (-1);
}

//  radius of an A2 base at its widest
#define MAXIMUM_HOME_RADIUS                 174.75
//  radius of a ping pong ball
#define BALL_RADIUS                         20
//  minimum boundary for object to to from rover to not need to avoid
#define MINIMUM_SAFE_OBJECT_X_DISPLACEMENT  70  //SHOULD BE SOMETHING LIKE 15
//  minimum boundary for rover from wall 
#define MINIMUM_SAFE_WALL_DISPLACEMENT      300
//  rover width
#define ROVER_WIDTH                         200

// type, displacement, angle 
int camera_readings_type = 0;
double camera_readings_displacemet = 0;
double camera_readings_angle = 0;

//  update values
  //  stash destination
double camera_stashed_x = 0;
double camera_stashed_y = 0;

//  counter, remember to mod 4 to reset it
int detourCounter = 0;

//  objects centre for reference
double object_x = 0;
double object_y = 0;

//  object radius
double object_radius = 0;

//  difference in space between the rover and the object
double object_rover_x_difference = 0;

//  how far away the object is when deciding how far to move away from it
double object_displacement = 0;
double object_angle = 0;

//  If the rover is in emergancy break procedure
int emergancy_corner_count = 0;
bool avoided = 0;

//  wall position
double wall_x = 0;
double wall_y = 0;

/////////////////////////////////////////////////////////////////
void updateTargets(double * B_x, double * B_y, double * current_x, double * current_y, double * current_angle, int * target_displacement, double * target_angle){
    
    double dx = *B_x - *current_x;
    double dy = *B_y - *current_y ;

    *target_angle = atan2( dy, dx ) - *current_angle;
    //  converts from angle to angle
      
    if((*target_angle) > PI){
        *target_angle -= 2 * PI;
    }
    else if ((*target_angle) < -PI){
        *target_angle += 2 * PI;
    }

   // update target displacement
    *target_displacement = (int) sqrt(pow(dy, 2) + pow(dx, 2));
//    Serial.println("Target Displacement: " + String(*target_displacement));
//    Serial.println("Final Angle: " + String(atan2( dy, dx )));
//    Serial.println("Target Angle: " + String(*target_angle));
//
//    Serial.println(*target_displacement * cos(*current_angle + *target_angle) + *current_x);
//    Serial.println(*target_displacement * sin(*current_angle + *target_angle) + *current_y);
}

/////////////////////////////////////////////////////////////////
//  automation parameters
  int counter = 0;
  #define ARENA_WIDTH         1050
  #define ARENA_HEIGHT        250
  #define SIDE_SECTION_SPANS  2
  #define MID_SECTION_SPANS   2
  bool skipLoop = 0;
  bool returning = 0;
  //  x_des is B_x
  //  y_des is B_y
  //  target_angle is target_angle
  //  x_pos is current_x
  //  y_pos is current_y
/*
  functin call exampleS
  automation(
      &counter,
      current_x, current_y,
      &B_x, &B_y, &target_angle
  )
*/

//  automation function
void automation(
    int * counter,
    double x_pos, double y_pos,
    double * x_des, double * y_des,/*, double * target_angle*/
    bool returning
)
{
    double x =   ARENA_WIDTH / (2*(2*SIDE_SECTION_SPANS + MID_SECTION_SPANS));
    double append = 0;
    x *= (
            ((*counter / 4) > SIDE_SECTION_SPANS) && 
            ((*counter / 4) < MID_SECTION_SPANS + SIDE_SECTION_SPANS)
        ) ? 
            (MID_SECTION_SPANS) : (SIDE_SECTION_SPANS);

    if(!returning){
        append = -x;
    }
    else{
        append = x;
    }
    if(*counter % 2 == 0){
        *x_des -= append;
    }
    else{
        if(*counter % 4 == 1){
            *y_des -= ARENA_HEIGHT;
        }
        else if(*counter % 4 == 3){
            *y_des += ARENA_HEIGHT;
        }
    }
        *counter += 1;
}

/////////////////////////////////////////////////////////////////

//  state mashine parameters for the drive process (stop turn go)
  int turning_complete = 1;
  int straight_line_complete = 1;

long lastCycle = 0;

/////////////////////////////////////////////////////////////////

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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    //Serial.println();
    //Serial.println();
    Serial.print("Waiting for WiFi... ");
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
        // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    Serial.println("\n");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(3000);
    /////////////////////////////////////////////////////////////////
    //  Connecting to TCP Server
    while (!client.connect(host, port)) {
        Serial.println("Connection to TCP Server failed");
        Serial.println("Trying again in 500ms...");
        delay(500);
    }
    Serial.println("Connected to Server\n");
    // // wait for mission start
    String start_message = "";
    while(client.available() < 5){
        Serial.println("Waiting for Mission Start");
        //client.print("{waiting: \"hi\"}");
        delay(500);
    }
    // tcp_received = client.readStringUntil('\r');
    // tcp_parse(tcp_received, &B_x, &B_y, &mode_);
    
    mode_ = "A";

    // if (mode_ == "A") {
    //   // update position to travel to
    //   automation(
    //       &counter,
    //       current_x, current_y,
    //       &B_x, &B_y, returning
    //   );
    // }

    // update target angle
    updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);

    Serial.println("counter: " + String(counter));
    Serial.println("cx: " + String(current_x));
    Serial.println("cy: " + String(current_y));
    Serial.println("bx: " + String(B_x));
    Serial.println("by: " + String(B_y));
    Serial.println("current angle: " + String(current_angle));   
    Serial.println("target displacement: " + String(target_displacement));
    Serial.println("target angle: " + String(target_angle));
    Serial.println("actual angle: " + String(target_angle));

    turning_complete = 1;
    straight_line_complete = 1;

}


/////////////////////////////////////////////////////////////////

void loop()
{

    // //-------- Measure Loop Period --------//
    // Serial.println(micros() - lastCycle);
    // lastCycle = micros();
    // //-----------------------------------------------//

    // //-------- Uncomment to ensure reliability --------//
    // // check if connected to WIFI
    // if (WiFi.status() == WL_CONNECTED) {
    //     // check if connected to server
    //     if (client.connected()) {
    //     } else if(millis() - wifi_last_connect_attempt > wifi_connect_timeout) {
    //     }
    // } else if (millis() - wifi_last_connect_attempt > wifi_connect_timeout) {
    //     WiFiMulti.run();
    //     wifi_connect_timeout = millis();
    // }
    // //-----------------------------------------------//

    current_x += 10;
    current_y += 10;
    if (current_x > 550 || current_y > 550) {
        current_x = 0;
        current_y = 0;
    }
    //-------- Periodically send data back to server --------//
    if (millis() - last_TCP_send > TCP_post_period) {
        tcp_send = "{\"time\":" + String(millis()) + 
                        ",\"type\": \"alien\"" + 
                        ",\"colour\": \"red\"," + 
                        "\"data\":"  + 
                        "{\"posX\": " + String(current_x) + 
                        ",\"posY\": " + String(current_y) + 
                        "}" + 
                        "}@";
        Serial.println("----------------------");
        Serial.println(tcp_send);
        Serial.println("----------------------");
        client.print(tcp_send);
        last_TCP_send = millis();
    }
    //-----------------------------------------------//
}
