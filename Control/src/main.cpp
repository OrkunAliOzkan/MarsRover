#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <vector>

// #define RADIUS 132
// /////////////////////////////////////////////////////////////////

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

// /////////////////////////////////////////////////////////////////

// Motor Controller Pin mappings
  #define PWMA 17
  #define PWMB 2
  #define AIN1 14
  #define AIN2 16
  #define BIN1 4
  #define BIN2 15

// /////////////////////////////////////////////////////////////////

//  Name of network
    #define WIFI_SSID       "ooo20"    
//  Password
    #define WIFI_PASSWORD   "naber_lan"
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

// /////////////////////////////////////////////////////////////////

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
const char * host = "146.169.171.197"; // ip or dns

// WiFi timeout variables (reliability)
  long wifi_last_connect_attempt = 0;
  long wifi_connect_timeout = 3000;
// tcp timeout variables (reliability)
  long tcp_last_connect_attempt = 0;
  long tcp_connect_timeout = 3000;
  
  long last_TCP_send = 0;
  long TCP_post_period = 50;

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

// Drive parameters
  #define MIN_ROTATION_PWM 90 
  #define MAX_ROTATION_PWM 220

  #define STRAIGHT_PWM_L 128
  #define STRAIGHT_PWM_R 154

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

// Destination coordinates
  double B_x = 1000;
  double B_y = 0;

// Current position and angle
  double current_x = 0;
  double current_y = 0;
  double current_angle = 0;

/////////////////////////////////////////////////////////////////

// PID for deviation (during straight line motion)
  // double Kp_deviation = 0.9;
  // double Ki_deviation = -0.1;
  // double Kd_deviation = 1.5;

  double Kp_deviation = 0.21;
  double Ki_deviation = 0.0002;
  double Kd_deviation = 0.005;

/////////////////////////////////////////////////////////////////

// // PID for displacement <- not used
//   double Kp_displacement = 3;
//   double Ki_displacement = 0;
//   double Kd_displacement = 0.05;

  int displacement_error = 0;
  
  //  PWM output
  int differential_PWM_output = 0;
  int target_displacement = 0;

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
  #define ARENA_WIDTH           1050
  #define ARENA_HEIGHT          250
  #define X_DISPLACEMENT_AMOUNT 300 //  TODO: Play with coefficients
  #define WIDTH_ERROR           200 //  TODO: Play with coefficients
  #define DISPLACEMENT_ERROR    150 //  TODO: Play with coefficients
  #define HEIGHT_ERROR          200 //  TODO: Play with coefficients
  #define Y_DISPLACEMENT_AMOUNT 300 //  TODO: Play with coefficients
  int state = 0;  
  int avoidance_counter = 0;        //  Is to determine the emergancy counter

//some function that creates an array
int automation(int state, double * destination_x, double * destination_y, double current_x, double current_y)
{
    switch(counter){
      case(0):{
          //  x
          *destination_x = current_x + X_DISPLACEMENT_AMOUNT;
          break;
      }
      case(1):{
          //  y
          *destination_y = current_y + (ARENA_HEIGHT /*- 2*RADIUS*/); //  have y avoid the walls by a rover width
          break;
      }
      case(2):{
          //  x
          *destination_x = current_x + X_DISPLACEMENT_AMOUNT;
          break;
      }
      case(3):{
          //  y
          *destination_y = current_y - (ARENA_HEIGHT /*- 2*RADIUS*/); //  have y avoid the walls by a rover width
          break;
      }
      default:{
          Serial.println("Shouldn't be here.");
          break;
      }        
    }
    counter++;
    counter %= 4;
    return 1;
}

/////////////////////////////////////////////////////////////////

//  obstacle avoidance

//  camera readings
bool camera_readings(int *camera_readings_type, double *camera_readings_displacemet, double *camera_readings_angle, double current_x, double current_y) {
  //  TEMPORARY OBJECT TO TEST AVOIDANCE
    double avoidance_x = 100;
    double avoidance_y = 100;

  *camera_readings_type = 7;

  //  object is at (100, 100)
  *camera_readings_displacemet = sqrt(pow(avoidance_x - current_x, 2) + pow(avoidance_y - current_y, 2));
  *camera_readings_angle = atan2(avoidance_y - current_y, avoidance_x - current_x);
  // return (*camera_readings_displacemet < 800);
  return 0;
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
int     camera_readings_type = 0;
double  camera_readings_displacemet = 0;
double  camera_readings_angle = 0;

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
    pinMode(PIN_SS,OUTPUT);   //(CHIP SELECT)
    pinMode(PIN_MISO,INPUT);  //(MASTER IN, SLAVE OUT)
    pinMode(PIN_MOSI,OUTPUT); //(MASTER OUT, SLAVE IN)
    pinMode(PIN_SCK,OUTPUT);  //(CLOCK)
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
    }

    Serial.println("\n");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);

    mode_ = "A";

    if (mode_ == "A") {
      // update position to travel to
      // automation(state, &B_x, &B_y, current_x, current_y);
    }

    // update target angle
    updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);

    Serial.println("counter: " + String(counter));
    Serial.println("cx: " + String(current_x));
    Serial.println("cy: " + String(current_y));
    Serial.println("bx: " + String(B_x));
    Serial.println("by: " + String(B_y));
    // Serial.println("current angle: " + String(current_angle));   
    // Serial.println("target displacement: " + String(target_displacement));
    // Serial.println("target angle: " + String(target_angle));
    // Serial.println("actual angle: " + String(target_angle));

    turning_complete = 1;
    straight_line_complete = 1;

}

/////////////////////////////////////////////////////////////////

void loop()
{
  Serial.println("Made it here");
    // if in auto mode and ready to travel to next waypoint
    if(current_x > 1000)
    {
      mode_ = "";
      turning_complete = 1;
      straight_line_complete = 1;
      
    }
    if((mode_ == "A") && (turning_complete) && (straight_line_complete)){

        // get next waypoint
        automation(state, &B_x, &B_y, current_x, current_y);
        // update target angle
        updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);

        Serial.println("////////////////////AUTO////////////////////");
        Serial.println("counter:\t" + String(counter));
        Serial.println("x_des:\t" + String(B_x));
        Serial.println("y_des:\t" + String(B_y));
        Serial.println("x_pos:\t" + String(current_x));
        Serial.println("y_pos:\t" + String(current_y));

        /*
        if(coodinate is same as final coodinate of automation_coodinate array){
          complete = 1;
        }
        */
            
//         turning_complete = 0;
//         straight_line_complete = 0;
//     }

    if (!turning_complete) {
        angular_error = (current_angle - target_angle);
        // rotation_deviation_error = total_path_y;

        if (abs(angular_error) < 0.018) {
            // brake
            analogWrite(PWMA, 0); 
            analogWrite(PWMB, 0);

//             turning_complete = 1;
//             differential_PWM_output = 0; 
            
            prevT = 0;
            
            // resetting OFS readings
            total_path_x = 0;
            total_path_y = 0;
        } else {
            // turning not complete
            currT = micros();
            deltaT = ((double) (currT-prevT))/1.0e6;
            
            current_angle += (target_angle > 0) ? (0.01) : (-0.01);
        }

        // Debugging Messages
        Serial.println("Rover is turning");
        Serial.println("target_angle: " + String(target_angle));
        Serial.println("angular error: " + String(angular_error));
        Serial.println("TOTAL_PATH_x: " + String(total_path_x));
        Serial.println("TOTAL_PATH_y: " + String(total_path_y));
        delay(10);
    } 
    else if (!straight_line_complete) {
            //  If there is an object inbound, needed to be avoided. TEMPORARY, KNOW IT NEEDS TO BE CHANGED
            if(camera_readings(&camera_readings_type, &camera_readings_displacemet, &camera_readings_angle, 
                                current_x, current_y)){  
              if( (avoidance_counter == 0) && 
                  (camera_readings_displacemet *cos(camera_readings_angle) < WIDTH_ERROR) && 
                  (camera_readings_displacemet *cos(camera_readings_angle) < DISPLACEMENT_ERROR)
                ){
              switch(state){
                case(0):{
                  B_y = (B_y - current_y > HEIGHT_ERROR) ? 
                          (min(B_y, current_y + Y_DISPLACEMENT_AMOUNT)) 
                          : 
                          (current_y + Y_DISPLACEMENT_AMOUNT);  
                  break;
                }
                case(1):{
                  B_x = (B_x - current_x > WIDTH_ERROR) ? 
                          (min(B_x, current_x + X_DISPLACEMENT_AMOUNT)) 
                          : 
                          (current_x + X_DISPLACEMENT_AMOUNT);                  
                  break;
                }
                case(2):{
                  B_y = (current_y - B_y > HEIGHT_ERROR) ? 
                          (min(B_y, current_y - Y_DISPLACEMENT_AMOUNT)) 
                          : 
                          (current_y + Y_DISPLACEMENT_AMOUNT);  
                  break; 
                }
                case(3):{
                  B_x = (B_x - current_x > WIDTH_ERROR) ? 
                          (min(B_x, current_x + X_DISPLACEMENT_AMOUNT)) 
                          : 
                          (current_x - X_DISPLACEMENT_AMOUNT);
                  break; 
                }
              }
              //  stop motors <---- TODO: Add this
              //  restart motion
              turning_complete = 1;
              straight_line_complete = 1;
              //  set to a value 
              avoidance_counter = 1;;
            }
              else if(avoidance_counter == 1){
              switch(state){
                case(0):{
                  B_x += X_DISPLACEMENT_AMOUNT;
                  break;
                }
                case(1):{
                  B_y = ARENA_WIDTH;
                  break;
                }
                case(2):{
                  B_x += X_DISPLACEMENT_AMOUNT;
                  break;
                }
                case(3):{
                  B_y = 0;
                  break;
                }
              }
              avoidance_counter = 0;
            }
          }


        displacement_error = target_displacement - total_path_y;
        
//         if (abs(displacement_error) <= 1) {
//             // brake when reached
//             analogWrite(PWMA, 0);  
//             analogWrite(PWMB, 0);

//             straight_line_complete = 1;
//             differential_PWM_output = 0; 

            // resetting PID variables
            prevT = 0;

//             // resetting total path readings
//             total_path_x = 0;
//             total_path_y = 0;

            //  adjusting the position SIMULATION ONLY
            current_x = B_x;
            current_y = B_y;


        } else {
            //  displacement error for debugging
            total_path_y += 1;
            delay(10);
        }

    //  debug content
        Serial.println("Rover moving in straight line:");
        Serial.println("p_term_angle: " + String(p_term_angle));
        Serial.println("i_term_angle: " + String(i_term_angle));
        Serial.println("d_term_angle: " + String(d_term_angle));
        Serial.println("Displacement Error: " + String(displacement_error));
        Serial.println("differential_PWM_output: " + String(differential_PWM_output));
        Serial.println("TOTAL_PATH_x: " + String(total_path_x));
        Serial.println("TOTAL_PATH_y: " + String(total_path_y));
        Serial.println("MotorSpeedA: " + String(MotorSpeedA));
        Serial.println("MotorSpeedB: " + String(MotorSpeedB));
        delay(100);
    }
    Serial.println("------------");
}