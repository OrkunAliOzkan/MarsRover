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
  float current_x = 100;
  float current_y = 100;
  float current_angle = -0.5*PI;

  float prev_x = current_x;
  float prev_y = current_y;
  float prev_angle = current_angle;

/////////////////////////////////////////////////////////////////


  #define RADIUS 132
//    #define RADIUS 120

//  min max PWM
  #define MIN_PWM 100
  #define MAX_PWM 200

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

  float p_term_angle;
  float i_term_angle;
  float d_term_angle;

  float differential_PWM_output = 0;
  float offset_PWM_output = 0;

  bool Rot_Ctrl = 0;//  Informed by server

/////////////////////////////////////////////////////////////////

//  PID for turning
  //  TODO: mess around with
  float Kp_rotation = 0.1 ;
  float Ki_rotation = 0;
  float Kd_rotation = 0;

/////////////////////////////////////////////////////////////////


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

//void OFS_Cartesian
//            (            
//            MD md, 
//            int * prescaled_tx, 
//            int * prescaled_ty, 
//            int * total_x, 
//            int * total_y
//            )
//{
//  //  Optical sensor readings
//    mousecam_read_motion(&md);
//
//  *prescaled_tx += convTwosComp(md.dx);
//  *prescaled_ty += convTwosComp(md.dy);
//
//  *total_x = *prescaled_tx / 4.95;
//  *total_y = *prescaled_ty / 4.95;
//
//  Serial.println("Dy: " + String(*prescaled_ty));
//  Serial.println("Dx: " + String(*prescaled_tx));
//  Serial.println("Total y: " + String(*total_y));
//  Serial.println("Total x: " + String(*total_x));
//  Serial.println();
//}

//void OFS_Angular(
//                MD md, 
//                float * total_x, 
//                float * total_y, 
//                float* abs_theta
//                )
//{
//      //  Optical sensor readings
//        mousecam_read_motion(&md);
//        *abs_theta += (convTwosComp(md.dx) / 4.95) / RADIUS;
//        float d_r = convTwosComp(md.dy) / 4.95;
//        *total_x += d_r * sin(*abs_theta);
//        *total_y += d_r * cos(*abs_theta);
//        Serial.println();
//        Serial.println("dx: " + String(md.dx));
//        Serial.println("dr: " + String(d_r));
//        Serial.println("d_theta: "+ String((convTwosComp(md.dx) / 4.95) / RADIUS));
//        Serial.println("Angle: " + String(*abs_theta));
//        Serial.println("Angle: " + String(*abs_theta));
//        Serial.println("Total y: " + String(*total_y));
//        Serial.println("Total x: " + String(*total_x));
//}

// double prescaled_tx = 0;
// double prescaled_ty = 0;
double total_path_x_R = 0;
double total_path_y_R = 0;
double x_coordinate_R = 0;
double y_coordinate_R = 0;
double angle_R = -PI/2;

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

//

//  update variables
  long currT = 0;
  long prevT = 0;
  float deltaT = 0;

/////////////////////////////////////////////////////////////////

//  tcp related variables
String tcp_received = "";
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
const char * host = "85.255.235.186"; // ip or dns

// WiFi timeout variables (reliability)
  long wifi_last_connect_attempt = 0;
  long wifi_connect_timeout = 3000;
// tcp timeout variables (reliability)
  long tcp_last_connect_attempt = 0;
  long tcp_connect_timeout = 3000;
  
  long last_TCP_post = 0;
  long TCP_post_period = 50;

// void updateTargets(float * B_x, float * B_y, float * current_x, float * current_y, float * current_angle, float * target_angle){
//     float dx = *B_x - *current_x;
//     float dy = *B_y - *current_y ;
//     *target_angle = atan2( dy, dx ) - *current_angle;
//     //  converts from angle to angle
//     if((*target_angle) > PI){
//         *target_angle -= 2 * PI;
//     }
//     else if ((*target_angle) < -PI){
//         *target_angle += 2 * PI;
//     }
// }

/////////////////////////////////////////////////////////////////

//  state mashine parameters for the drive process (stop turn go)
  int turning_complete = 0;

long lastSampleT = 0;
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
    //  Connecting to TCP Server
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
    // // tcp_parse(tcp_received, &B_x, &B_y, &mode_);
    // update target angle
    // updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_angle);
    //  hard code target angle
    //target_angle = 6*PI;
      turning_complete = 0;
//     // digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
//     // digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
// //    analogWrite(PWMA, 200); 
// //    analogWrite(PWMB, 200);
    lastSampleT = 0;
    // set the right motor directions

    // digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
    // digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
    // // power the motors
    // analogWrite(PWMA, 120);  
    // analogWrite(PWMB, 126.5);
}

String location_info = "";

long samplePeriod = 0;

long tcp_send_prev = 0;
void loop()
{
  if(turning_complete)
  {
    target_angle *= -1;
    turning_complete = 0;
    delay(1000);
  }

    OFS_Readings
            (            
            md, 
            &total_path_x_R, 
            &total_path_y_R, 
            &x_coordinate_R, 
            &y_coordinate_R,
            &angle_R
            );
    Serial.println("abs(angle_R) - target_angle:\t" + String(0.75*abs(angle_R - target_angle)));
    samplePeriod = micros() - lastSampleT;
    lastSampleT = micros();
    Serial.println();
    Serial.println(samplePeriod);

    //delay(50);
    // Periodically send data back to server

    if (!turning_complete) {
        // Rotation Logic
        // OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        //OFS_Angular(md, &totalpath_x_flt, &totalpath_y_flt, &abs_theta);
        // Serial.println("-------");
        // Serial.println("totalpath_x_int:\t" + String(totalpath_x_int));
        // Serial.println("totalpath_y_int:\t" + String(totalpath_y_int));
        // Serial.println("-------");
        // offset_error = (total_path_x_R - RADIUS*target_angle);

        offset_error = total_path_y_R;

        // Serial.println("in !turning_complete");
        // simplistic dead reckoning
        //current_angle = ((float) totalpath_x_int) / RADIUS + prev_angle;
          Serial.println("offset_error:\t" + String(angle_R - target_angle));
        if (0.9*abs(angle_R - target_angle) < 0.05) {
            Serial.println("in error good");
            // brake
            analogWrite(PWMA, 0); 
            analogWrite(PWMB, 0);

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
            
            // simplistic dead reckoning
            //prev_angle = current_angle;
        } else {
          Serial.println("in else");
          Serial.println("differential_PWM_output:\t" + String(differential_PWM_output));
            // turning not complete
            currT = micros();
            deltaT = ((float) (currT-prevT))/1.0e6;
            
            p_term_angle = offset_error;
            i_term_angle += offset_error * deltaT;
            d_term_angle = (offset_error - offset_error_prev)/deltaT;

            offset_PWM_output = abs(Kp_rotation * p_term_angle + Ki_rotation * i_term_angle);

            // guards to keep output within bounds
              differential_PWM_output = MIN_PWM;

            // set the right motor directions
            if (target_angle > 0) {
                digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
                digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
            } else {
                digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
                digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
            }

            /*
              if (offset_error <= 0) {
                digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
                digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
            // } else {
                digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
                digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
            }

            // power the motors
            analogWrite(PWMA, differential_PWM_output + (offset_error <= 0) ? (- offset_PWM_output) :(offset_PWM_output);  
            analogWrite(PWMB, differential_PWM_output + (offset_error <= 0) ? (offset_PWM_output) :(-offset_PWM_output);
            */

            // power the motors
            analogWrite(PWMA, differential_PWM_output - offset_PWM_output);  
            analogWrite(PWMB, differential_PWM_output + offset_PWM_output);

            // update variables for next cycle
            prevT = currT;
            offset_error_prev = offset_error;
        }
    }
    delay(30);
}
