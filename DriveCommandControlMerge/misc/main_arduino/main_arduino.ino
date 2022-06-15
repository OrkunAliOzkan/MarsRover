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
    #define WIFI_SSID       "bet-hotspot"    
//  Password
    #define WIFI_PASSWORD   "helloworld"
//  Name of network
//    #define WIFI_SSID       "CommunityFibre10Gb_003D7"    
//  Password
//    #define WIFI_PASSWORD   "gxqxs3c3fs" 
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
double A_x = 0; // TODO: Define at end of movement
double A_y = 0; // TODO: Define at end of movement

// Destination coordinates
double B_x = 0;
double B_y = 750;

// Current position and bearing
double current_x = 100;
double current_y = 100;
double current_bearing = 0;

/////////////////////////////////////////////////////////////////

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
float Ki_rotation = 0.2;
float Kd_rotation = 0.2;

/////////////////////////////////////////////////////////////////

// PID for straight line
float Kp_deviation = 3;
float Ki_deviation = 0.05;
float Kd_deviation = 0.05;

struct MD
{
    byte motion;
    char dx, dy;
    byte squal;
    word shutter;
    byte max_pix;
};

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

  //Serial.println("Dy: " + String(convTwosComp(md.dy)));
  //Serial.println("Dx: " + String(convTwosComp(md.dx)));
  //Serial.println("Total y: " + String(*total_y));
  //Serial.println("Total x: " + String(*total_x));
  //Serial.println();
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
        *abs_theta += (convTwosComp(md.dx) / 4.95) / RADIUS ;
        float d_r = convTwosComp(md.dy) / 4.95;
        *total_x += d_r * sin(*abs_theta);
        *total_y += d_r * cos(*abs_theta);
        //Serial.println("dr: " + String(d_r));
        //Serial.println("d_theta: "+ String((convTwosComp(md.dx) / 4.95) / RADIUS));
        //Serial.println("Angle: " + String(*abs_theta));
        //Serial.println("Total y: " + String(*total_y));
        //Serial.println("Total x: " + String(*total_x));
}

long currT = 0;
long prevT = 0;
float deltaT = 0;

/////////////////////////////////////////////////////////////////

int differential_PWM_output;

/////////////////////////////////////////////////////////////////
// PID for displacement
/*
float Ki_displacement = 0;
float Kp_displacement = 3;
float Kd_displacement = 0.05;
*/
int displacement_PWM_output = 128;  //  Displacement controllers PWM output
int error_displacement = 0;
int target_displacement = 0;

MD md;
/////////////////////////////////////////////////////////////////
String post_data = "";

int arrived = 0;
int turning_arrived = 0;
/////////////////////////////////////////////////////////////////
//  automation
  /*
  void fclass::automation(
      int * counter,
      float arena_width, float arena_height,
      int side_sections_spans, int mid_sections_spans,
      float x_pos, float y_pos,
      float * x_des, float * y_des, float * target_bearing
  )
  {
      {
          int sign = ((*counter % 4 == 0) || (*counter % 4 == 1)) 
                      ? (1) : (-1);
          *target_bearing = sign * 90 * (*counter != 0);
      }
      {
          //  Determining x
          {
              int DisplaceByX = 0;
              DisplaceByX = (int)(
                      ((*counter) % 2 != 0))
                      * (arena_width);
              DisplaceByX *= (
                  ( ( (*counter) % 3) == 0 ) ? (-1) : (1)
                  );
              *x_des = x_pos + DisplaceByX;
          }
          //  Determining y
          {
              int DisplaceByY = 0;
              float scalingFactor = 0;
              scalingFactor = (
                      (*counter < 2*side_sections_spans) || 
                      (*counter > 2 * (mid_sections_spans + side_sections_spans))) ? 
                      (side_sections_spans) : (mid_sections_spans);

              DisplaceByY = (int)(*counter % 2 == 0) * (arena_height / (2*scalingFactor));
              *y_des = y_pos + DisplaceByY;
          }
          *counter += 1;
      }
  }
  */

void setup()
{
  Serial.begin(115200);
  Wire.begin();
/////////////////////////////////////////////////////////////////
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
/////////////////////////////////////////////////////////////////
  pinMode(PIN_SS,OUTPUT); //(CHIP SELECT)
  pinMode(PIN_MISO,INPUT); //(MASTER IN, SLAVE OUT)
  pinMode(PIN_MOSI,OUTPUT); //(MASTER OUT, SLAVE IN)
  pinMode(PIN_SCK,OUTPUT); //(CLOCK)
/////////////////////////////////////////////////////////////////
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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Initializing");

  delay(3000);
  turning_arrived = 1;
  arrived = 1;
}

int rotation_tuning = 1;
int straight_line_tuning = 1;

bool command_received = 1;
bool testingifread = 0;

int turning_complete = 1;
int straight_line_complete = 1;
void loop()
{
    post_data = "";
    if (turning_complete && straight_line_complete) {
        target_angle = (45.0 / 180.0) * PI;
        A_y = 0;
        A_x = 0;
        B_y = 1000;
        B_x = 0;
        target_displacement = sqrt(pow(B_y - A_y, 2) + pow(B_x - A_x, 2));
        turning_complete = 0;
        straight_line_complete = 0;
    }
    if (!turning_complete) {
        OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        angular_error = (totalpath_x_int - RADIUS*target_angle);

        if (abs(angular_error) < 3) {
            // brake
            analogWrite(PWMA, 0); 
            analogWrite(PWMB, 0);

            turning_complete = 1;
            differential_PWM_output = 0; 
            
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

            post_data = "-------------------------------------------------------\n";
            post_data += ("Turning Complete\n");
        } else {
            // turning not complete
            currT = micros();
            deltaT = ((float) (currT-prevT))/1.0e6;
            
            p_term_angle = angular_error;
            i_term_angle += angular_error * deltaT;
            d_term_angle = (angular_error - angular_error_prev)/deltaT;

            differential_PWM_output = abs(Kp_rotation * p_term_angle);

            // guards to keep output within bounds
            if (differential_PWM_output > 255) {
                differential_PWM_output = 255;
            }
            else if (differential_PWM_output < 38) {
                differential_PWM_output = 38;
            }

            // set the right motor directions
            if (angular_error <= 0) {
                digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
                digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
            } else {
                digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
                digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
            }

            // power the motors
            analogWrite(PWMA, differential_PWM_output);  
            analogWrite(PWMB, differential_PWM_output);

            // update variables for next cycle
            prevT = currT;
            angular_error_prev = angular_error;
        }

        // Debugging Messages
        post_data += "-------------------------------------------------------\n";
        post_data +=("Rover is turning");
        post_data += "\n";
        post_data +=("target_angle: " + String(target_angle));
        post_data += "\n";
        post_data +=("angular error: " + String(angular_error));
        post_data += "\n";
        post_data +=("deltaT: " + String(deltaT));
        post_data += "\n";
        post_data +=("P: " + String(p_term_angle * Kp_rotation));
        post_data += "\n";
        post_data +=("I: " + String(i_term_angle * Ki_rotation));
        post_data += "\n";
        post_data +=("D: " + String(d_term_angle * Kd_rotation));
        post_data += "\n";
        post_data +=("differential_PWM_output: " + String(differential_PWM_output));
        post_data += "\n";
        post_data +=("TOTAL_PATH_x: " + String(totalpath_x_int));
        post_data += "\n";
        post_data +=("TOTAL_PATH_y: " + String(totalpath_y_int));
        post_data += "\n";

    } else if (!straight_line_complete) {
    
        OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        error_displacement = target_displacement - totalpath_y_int;
        
        if (abs(error_displacement) < 10) {
            // brake when reached
            analogWrite(PWMA, 0);  
            analogWrite(PWMB, 0);

            straight_line_complete = 1;
            displacement_PWM_output = 0;
            differential_PWM_output = 0; 
            
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

            post_data = "-------------------------------------------------------\n";
            post_data += ("Straight Line Complete\n");
        } else {
            //  y-axis pid controller
            /*
            displacement_PWM_output = Kp_displacement * error_displacement;
            //  guards
            displacement_PWM_output = (displacement_PWM_output > 220) ? (220) : (displacement_PWM_output);
            displacement_PWM_output = (displacement_PWM_output < 38) ?  (38)  : (displacement_PWM_output);
            //  strictly for testing purposes FIXME: DELETE
            MotorSpeedA = displacement_PWM_output;
            MotorSpeedB = displacement_PWM_output;
            */

            // deviation pid controller
            currT = micros();
            deltaT = ((float) (currT-prevT))/1.0e6;

            angular_error = (totalpath_x_int);

            p_term_angle = (angular_error);
            i_term_angle += (angular_error*deltaT);
            //    d_term_angle = (angular_error - angular_error_prev)/deltaT;
            differential_PWM_output = p_term_angle * Kp_deviation + i_term_angle * Ki_deviation; //0.3 is good, 0.33 decent

            MotorSpeedA = displacement_PWM_output + differential_PWM_output;
            MotorSpeedB = displacement_PWM_output - differential_PWM_output;

        //  guards
            if (MotorSpeedA < 0) {MotorSpeedA = 0;}
            if (MotorSpeedA > 255) {MotorSpeedA = 255;}

            if (MotorSpeedB < 0) {MotorSpeedB = 0;}
            if (MotorSpeedB > 255) {MotorSpeedB = 255;}

        //  inform the motors which way they are rotating
            digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CW  // ACW Rover
            digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CCW

        //  write PWMs to the motors
            analogWrite(PWMA, MotorSpeedA);  
            analogWrite(PWMB, MotorSpeedB);    

        //  update variables for next cycle
            angular_error_prev = angular_error;
            prevT = currT;
        }

    //  debug content
        post_data +="-------------------------------------------------------\n";
        post_data +=("Angular Error: " + String(angular_error));
        post_data += "\n";
        post_data +=("differential_PWM_output: " + String(differential_PWM_output));
        post_data += "\n";
        post_data +=("TOTAL_PATH_x: " + String(totalpath_x_int));
        post_data += "\n";
        post_data +=("TOTAL_PATH_y: " + String(totalpath_y_int));
        post_data += "\n";
        post_data +=("MotorSpeedA: " + String(MotorSpeedA));
        post_data += "\n";
        post_data +=("MotorSpeedB: " + String(MotorSpeedB));
        post_data += "\n";
    }

    Serial.println(post_data);
}
