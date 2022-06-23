// PIO includes
  #include <SPI.h>
  #include <Wire.h>
  #include <WiFi.h>

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
    #define WIFI_SSID       "ooo20"    
//  Password
    #define WIFI_PASSWORD   "naber_lan"

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
  float B_y = 0;

// Current position and angle
  float current_x = 0;
  float current_y = 0;
  float current_angle = 0;

  float prev_x = current_x;
  float prev_y = current_y;
  float prev_angle = current_angle;

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
  float offset_error = 0;
  float offset_error_prev = 0;
  float target_angle = 0;  // target angle, starting from 90 degrees

  float angular_error = 0;
  float angular_error_prev = 0;

  float p_term_angle;
  float i_term_angle;
  float d_term_angle;

  float offset_PWM_output = 0;

  bool Rot_Ctrl = 0;//  Informed by server

/////////////////////////////////////////////////////////////////

//  PID for turning
  float Kp_rotation = 4;
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

  Serial.println("Dy: " + String(*prescaled_ty));
  Serial.println("Dx: " + String(*prescaled_tx));
  Serial.println("Total y: " + String(*total_y));
  Serial.println("Total x: " + String(*total_x));
  Serial.println();
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
        //Serial.println("dr: " + String(d_r));
        //Serial.println("d_theta: "+ String((convTwosComp(md.dx) / 4.95) / RADIUS));
        //Serial.println("Angle: " + String(*abs_theta));
        //Serial.println("Total y: " + String(*total_y));
        //Serial.println("Total x: " + String(*total_x));
}

//  update variables
  long currT = 0;
  long prevT = 0;
  float deltaT = 0;

/////////////////////////////////////////////////////////////////
//  obstacle avoidance

//  camera readings
void camera_readings(int *camera_readings_type, float *camera_readings_displacemet, float *camera_readings_angle) {
*camera_readings_type = 7;

//  object is at (30, 30)
*camera_readings_displacemet = 27;
*camera_readings_angle = -PI/2;
}

int sign(float number){
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
float camera_readings_displacemet = 0;
float camera_readings_angle = 0;

//  update values
  //  stash destination
float camera_stashed_x = 0;
float camera_stashed_y = 0;

//  counter, remember to mod 4 to reset it
int detourCounter = 0;

//  objects centre for reference
float object_x = 0;
float object_y = 0;

//  object radius
float object_radius = 0;

//  difference in space between the rover and the object
float object_rover_x_difference = 0;

//  how far away the object is when deciding how far to move away from it
float object_displacement = 0;
float object_angle = 0;

//  If the rover is in emergancy break procedure
int emergancy_corner_count = 0;
bool avoided = 0;

//  wall position
float wall_x = 0;
float wall_y = 0;

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
// String tcp_send = "";
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

WiFiClient client; // Use WiFiClient class to create TCP connections
const uint16_t port = 8080;
const char * host = "146.169.168.102"; // ip or dns

// WiFi timeout variables (reliability)
  long wifi_last_connect_attempt = 0;
  long wifi_connect_timeout = 3000;
// tcp timeout variables (reliability)
  long tcp_last_connect_attempt = 0;
  long tcp_connect_timeout = 3000;
  
  long last_TCP_post = 0;
  long TCP_post_period = 50;

void updateTargets(float * B_x, float * B_y, float * current_x, float * current_y, float * current_angle, int * target_displacement, float * target_angle){
    
    float dx = *B_x - *current_x;
    float dy = *B_y - *current_y ;

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
     float x_pos, float y_pos,
     float * x_des, float * y_des,/*, float * target_angle*/
     bool returning
 )
 {
     float x =   ARENA_WIDTH / (2*(2*SIDE_SECTION_SPANS + MID_SECTION_SPANS));
     float append = 0;
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

//void automation(
//    int * counter,
//    float x_pos, float y_pos,
//    float * x_des, float * y_des,/*, float * target_angle*/
//    bool returning
//)
//{
//    float x =   ARENA_WIDTH / (2*(2*SIDE_SECTION_SPANS + MID_SECTION_SPANS));
//    float append = 0;
//    x *= (
//            ((*counter / 4) > SIDE_SECTION_SPANS) && 
//            ((*counter / 4) < MID_SECTION_SPANS + SIDE_SECTION_SPANS)
//        ) ? 
//            (MID_SECTION_SPANS) : (SIDE_SECTION_SPANS);
//
//    if(!returning){
//        append = -x;
//    }
//    else{
//        append = x;
//    }
//    if(*counter % 2 == 0){
//        *x_des -= append;
//    }
//    else{
//        if(*counter % 4 == 1){
//            *y_des += ARENA_HEIGHT;
//        }
//        else if(*counter % 4 == 3){
//            *y_des -= ARENA_HEIGHT;
//        }
//    }
//        *counter += 1;
//}

/////////////////////////////////////////////////////////////////

//  OFS Readings
double total_path_x_R = 0;
double total_path_y_R = 0;
double x_coordinate_R = 0;
double y_coordinate_R = 0;
double angle_R = 0;

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
    *total_path_x += convTwosComp(md.dx) / 4.95;
    *total_path_y += convTwosComp(md.dy) / 4.95;

    double tmp_angle = *angle + (convTwosComp(md.dx) / 4.95) / RADIUS / 2;
    *angle += (convTwosComp(md.dx) / 4.95) / RADIUS;

    *x_coordinate += dy * cos(tmp_angle);
    *y_coordinate += dy * sin(tmp_angle);
/*
    Serial.println("Dy: " + String(dy));
    Serial.println("Dx: " + String(dx));
     Serial.println("Angle: " + String(*angle));
    Serial.println("Total path y: " + String(*total_path_y));
    Serial.println("Total path x: " + String(*total_path_x));
    Serial.println("y_coordinate: " + String(*y_coordinate));
    Serial.println("x_coordinate: " + String(*x_coordinate));
    Serial.println();
*/
}

/////////////////////////////////////////////////////////////////

//  state mashine parameters for the drive process (stop turn go)
int turning_complete = 1;
int straight_line_complete = 1;

long lastCycle = 0;
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
    }

    Serial.println("\n");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);

    mode_ = "A";

    if (mode_ == "A") {
      // update position to travel to
    automation(
        &counter,
        current_x, current_y,
        &B_x, &B_y, returning
    );

    }
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

    turning_complete = 0;
    straight_line_complete = 0;
}

String location_info = "";

void loop()
{
    Serial.println(micros() - lastCycle);
    lastCycle = micros();

    if((mode_ == "A") && turning_complete && straight_line_complete){
        // brake

        // return condition
        if(current_x > ARENA_WIDTH)
        {    
            // head back to base
            returning = 1;
            //  ensures not following the original path but one slightly offset
            B_x = current_x - 
                (SIDE_SECTION_SPANS * ARENA_WIDTH) / 
                (2*(2*SIDE_SECTION_SPANS + 
                        MID_SECTION_SPANS));
        }
        // only for testing
        else if(current_x < 100)
        {    
            returning = 0;
        }

        // if(!skipLoop){
            automation(
                &counter,
                current_x, current_y,
                &B_x, &B_y, returning
            );

        //  update target angle
        updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);
        
        Serial.println("counter:\t" + String(counter));
        Serial.println("x_des:\t" + String(B_x));
        Serial.println("y_des:\t" + String(B_y));
        Serial.println("x_pos:\t" + String(current_x));
        Serial.println("y_pos:\t" + String(current_y));
        Serial.println("target_angle:\t" + String(target_angle));
        Serial.println("angle:\t" + String(angle_R));
            
        turning_complete = 0;
        straight_line_complete = 0;
    }

    if (!turning_complete) {
        // Rotation Logic

        angle_R = target_angle;

       if (0.9*abs(angle_R - target_angle) < 0.05) {
            Serial.println("in error good");
            // brake

            total_path_x_R = 0;
            total_path_y_R = 0;

            turning_complete = 1;
            differential_PWM_output = 0; 
            offset_PWM_output = 0; 
            
            // resetting PID variables
            prevT = 0;
            p_term_angle = 0;
            i_term_angle = 0;
            d_term_angle = 0;
            // resetting OFS_Cartesian variables
            prescaled_tx = 0;
  //            prescaled_ty = 0;
            totalpath_x_int = 0;
            
        }
        delay(30);
    }

    else if (!straight_line_complete) {
        displacement_error = 0;
        
        if (abs(displacement_error) < 4) {
            // brake when reached
            analogWrite(PWMA, 0);  
            analogWrite(PWMB, 0);

            straight_line_complete = 1;
            displacement_PWM_output = 0;
            differential_PWM_output = 0; 

            // simplistic dead reckoning
            prev_x = current_x;
            prev_y = current_y;
            
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

            // tcp_send = "---\n";
            // tcp_send += ("Straight Line Complete\n");
        }
        delay(1000);
  }
}
