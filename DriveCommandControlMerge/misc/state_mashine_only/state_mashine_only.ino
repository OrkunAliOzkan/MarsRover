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
  float angular_error = 0;
  float angular_error_prev = 0;
  float target_angle = 0;

  float p_term_angle;
  float i_term_angle;
  float d_term_angle;

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
const char * host = "146.169.171.197"; // ip or dns

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
    /////////////////////////////////////////////////////////////////
    // //  Connecting to TCP Server
    // if (!client.connect(host, port)) {
    //     Serial.println("Connection to TCP Server failed");
    //     Serial.println("Trying again in 500ms...");
    //     delay(500);
    // }
    // Serial.println("Connected to Server\n");
    // // // wait for mission start
    // String start_message = "";
    // while(client.available() < 5){
    //     Serial.println("Waiting for Mission Start");
    //     delay(500);
    // }
    // tcp_received = client.readStringUntil('\r');
    // tcp_parse(tcp_received, &B_x, &B_y, &mode_);
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
    //Serial.println(micros() - lastCycle);
    lastCycle = micros();
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
    //Serial.println(tcp_send);
    // tcp_send = "";

    // // Periodically send data back to server
    // if (millis() - last_TCP_post > TCP_post_period) {
    //     // Serial.println(tcp_send);
    //     location_info = "{\"time\":" + String(millis()) + 
    //                     ",\"type\": \"rover\"," + 
    //                     "\"data\":"  + 
    //                     "{\"posX\": " + String(current_x) + 
    //                     ",\"posY\": " + String(current_y) + 
    //                     ",\"angle\": " + String(current_angle) + 
    //                     "}" + 
    //                     "}";

    //     Serial.println("----------------------");
    //     Serial.println(location_info);
    //     Serial.println("----------------------");
    //     client.print(location_info);
    //     last_TCP_post = millis();
    // }

    // checks if there is a message from server in buffer
    if ((mode_ == "M") && (client.available() > 5)) {
        // brake
        analogWrite(PWMA, 0); 
        analogWrite(PWMB, 0);
        //read back one line from the server
        tcp_received = "";
        char c = client.read();
        while(c != 255){
            tcp_received += c;
            c = client.read();
        }

        //  parse data recieved
        tcp_parse(tcp_received, &B_x, &B_y, &mode_);

        //  update target angle
        updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);
        
        
        Serial.println("////////////////////MANU////////////////////");

        Serial.println("cx: " + String(current_x));
        Serial.println("cy: " + String(current_y));
        Serial.println("bx: " + String(B_x));
        Serial.println("by: " + String(B_y));
        Serial.println("current angle: " + String(current_angle));   
        Serial.println("target displacement: " + String(target_displacement));
        Serial.println("target angle: " + String(target_angle));

        //delay(5000);
        turning_complete = 0;
        straight_line_complete = 0; 
    }

    else if((mode_ == "A") && (turning_complete) && (straight_line_complete)){
        // brake
        analogWrite(PWMA, 0); 
        analogWrite(PWMB, 0);

        if(current_x > 1050)
        {    
            returning = 1;
            B_x = current_x - 
                (SIDE_SECTION_SPANS * ARENA_WIDTH) / 
                (2*(2*SIDE_SECTION_SPANS + 
                        MID_SECTION_SPANS));
        }
        else if(current_x < 100)
        {    
            returning = 0;

        }
        if(!skipLoop){
            automation(
                &counter,
                current_x, current_y,
                &B_x, &B_y, returning
            );
        }
        else{
            B_x -= (ARENA_WIDTH * SIDE_SECTION_SPANS) / (4*(2*SIDE_SECTION_SPANS + MID_SECTION_SPANS));
            skipLoop = 0;
        }

        //  update target angle
        updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);
        
        Serial.println("counter:\t" + String(counter));
        Serial.println("x_des:\t" + String(B_x));
        Serial.println("y_des:\t" + String(B_y));
        Serial.println("x_pos:\t" + String(current_x));
        Serial.println("y_pos:\t" + String(current_y));
        Serial.println("target_angle:\t" + String(target_angle));
            
        turning_complete = 0;
        straight_line_complete = 0;
    }
/*
  //  camera readings
    camera_readings(&camera_readings_type, &camera_readings_displacemet, &camera_readings_angle);

  //  update values
    object_x = current_x + (camera_readings_displacemet*sin( (PI * camera_readings_angle) / 180 ));
    object_y = current_y + (camera_readings_displacemet*cos( (PI * camera_readings_angle) / 180 ));

    object_radius = (camera_readings_type == 7) ? (MAXIMUM_HOME_RADIUS) : (BALL_RADIUS);

    object_rover_x_difference = abs(object_x - current_x) /*+ object_radius - (ROVER_WIDTH / 2);

    object_displacement = sqrt(pow((object_x - current_x), 2) + pow((object_y - current_y), 2));
    object_angle = atan2(object_y - current_y, object_x - current_x) - current_angle;

    //  if there is an object and it is less than 10cm away
    if(  
        (camera_readings_type > 0) &&
        (object_displacement < 100) &&
        (object_rover_x_difference < MINIMUM_SAFE_OBJECT_X_DISPLACEMENT) && 
        (emergancy_corner_count == 0) &&
        (!avoided)){
          Serial.println("HELLo");
      //  if haven't already emergancy breaked
      //  stop
          analogWrite(PWMA, 0); 
          analogWrite(PWMB, 0);

        //  stash desired destination coodinate
        camera_stashed_x = B_x;
        camera_stashed_y = B_y;

        //  tell rover to move left or right in opposite direction to the object
        //  mximum amount needed to move out by eye would probably be 2 of the object displacements.
        // Serial.println(object_angle >  0);
        // Serial.println(abs(camera_stashed_x - current_x - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT);
        B_x = (object_angle >  0) ? 
        (
          (abs(camera_stashed_x - current_x - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT) ? (3*object_radius) : (-2*object_radius)
        ) : 
        (
          (abs(camera_stashed_x - current_x - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT) ? (-3*object_radius) : (2*object_radius)
        ); 

        // B_x = (abs(camera_stashed_x - current_x - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT) ? 
        //       (sign(camera_stashed_x - current_x - object_displacement - 2*object_radius)*(2*object_radius)) 
        //       : 
        //       (camera_stashed_x);
        // break;

        B_y = current_y;

        // Serial.println(
        // "\nemergancy_corner_count:\t" + String(emergancy_corner_count) +
        // "\nB_x:\t" + String(B_x) +
        // "\nB_y:\t" + String(B_y) +
        // "\nobject_angle:\t" + String(object_angle)
        // );

        // Serial.println("object_displacement:\t" + String(object_displacement));
        // Serial.println("abs(camera_stashed_y - current_y - object_displacement - 2*object_radius):\t" + String(abs(camera_stashed_y - current_y - object_displacement)));

        emergancy_corner_count = 1;
        //  initialise the emergancy procedure
    }
    //  0)
    else if((emergancy_corner_count > 0) && (!avoided)){
            if((turning_complete) && (straight_line_complete)){
            //  stop
            analogWrite(PWMA, 0); 
            analogWrite(PWMB, 0);

            switch(emergancy_corner_count)
            {
                case 1:{
                // Serial.println("1\n");
                //  tell rover to move forward or backward in direction to the object
                //  mximum amount needed to move out by eye would probably be 2 of the object displacements.

                //abs(camera_stashed_y - current_y - object_displacement 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT

                if(abs(camera_stashed_y - current_y - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT){
                  // Serial.println("Yo");
                    B_y = camera_stashed_y;
                    emergancy_corner_count = 0;
                    avoided = 1;
                  break; //  equivalent to continue-ing the loop in c++
                }
                else{
                  B_y = (sign(current_angle) * (2*object_radius));
                }
                break;
                }

                case 2:{
                // Serial.println("2\n");
                //  return to original x position
                B_x = (abs(camera_stashed_x - current_x - object_displacement - 2*object_radius) < MINIMUM_SAFE_WALL_DISPLACEMENT) ? 
                (sign(camera_stashed_x - current_x - object_displacement - 2*object_radius)*(2*object_radius)) 
                : 
                (camera_stashed_x);
                break;

                }

                case 3:{
                // Serial.println("3\n");
                //  return to travelling on original path
                B_y = camera_stashed_y;
                avoided = 1;
                break;

                }
            }
            emergancy_corner_count++;
            emergancy_corner_count = emergancy_corner_count % 4;
            turning_complete = 0;
            straight_line_complete = 0;

//            Serial.println(
//            "\nemergancy_corner_count:\t" + String(emergancy_corner_count) +
//            "\nB_x:\t" + String(B_x) +
//            "\nB_y:\t" + String(B_y) +
//            "\nobject_angle:\t" + String(object_angle)
//            );
            }
    }

*/
    if (!turning_complete) {
        current_angle = ((float) totalpath_x_int) / RADIUS + prev_angle;
        turning_complete = 1;

        // Debugging Messages
        Serial.println("Rover has turned");
    } 
    else if (!straight_line_complete) {
        current_x = B_x;
        current_y = B_y;
        
        straight_line_complete = 1;

    //  debug content
        Serial.println("Rover moved in straight line:");
    }
    delay(1000);
}
