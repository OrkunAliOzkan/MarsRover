#include <Wire.h>
#include <SPI.h>
#define RADIUS 122
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
#define PWMA 17
#define PWMB 2
#define AIN1 14
#define AIN2 16
#define BIN1 4
#define BIN2 15
/////////////////////////////////////////////////////////////////
//  Drive parameters
float A_x = 0;
float A_y = 0;
float B_x = 0;
float B_y = 750;
int MotorSpeedA = 0; //  Final input to motors
int MotorSpeedB = 0; //  Final input to motors
int Bang_Constant = 128;
/////////////////////////////////////////////////////////////////
float ABSOLUTE_ANGLE = 0;
float CURR_x = 0;
float CURR_y = 0;
/////////////////////////////////////////////////////////////////

struct MD
{
 byte motion;
 char dx, dy;
 byte squal;
 word shutter;
 byte max_pix;
};
MD md;
//  Functions
int convTwosComp(int b)
{
    //Convert from 2's complement
    if(b & 0x80)
    {
      b = -1 * ((b ^ 0xff) + 1);
    }
    return b;
}

///////////////////////////////////////////////////////////////////////////////////
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

  Serial.println("Dy: " + String(convTwosComp(md.dy)));
  Serial.println("Dx: " + String(convTwosComp(md.dx)));
  Serial.println("Total y: " + String(*total_y));
  Serial.println("Total x: " + String(*total_x));
  Serial.println();
}

void x_displacement
                (
                    float *CURR_x,
                    float *CURR_y,
                    float *A_x,
                    float *A_y,
                    float *B_x,
                    float *B_y,
                    float *error_angle
                )
{
  float x = (float)(((*B_x - *CURR_x)*(*B_x - *A_x) + (*B_y - *CURR_y)*(*B_y - *A_y))/
                    (sqrt( pow((*B_x - *CURR_x), 2) + pow((*B_y - *CURR_y), 2)) * 
                    sqrt( pow((*B_x - *A_x), 2) + pow((*B_y - *A_y), 2))));
    //  Section which determines the angle
    //  For now just using projection
    *error_angle = PI / 2 - x - pow(x, 3)/6 - 5*pow(x, 5)/40;
}
///////////////////////////////////////////////////////////////////////////////////
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
//  Optical Flow Sensor parameters
int prescaled_tx = 0;
int prescaled_ty = 0;
int totalpath_x_int = 0; // prescaling x displacement
int totalpath_y_int = 0; // prescaling y displacement

float abs_theta = 0;
float totalpath_x_flt = 0; // postscaling x displacement
float totalpath_y_flt = 0; // postscaling y displacement
void OFS_Angular(MD md, float * total_x, float * total_y, float* abs_theta)
{
  //  Optical sensor readings
  // delay(1);
  mousecam_read_motion(&md);
  // theta += d_theta 
  *abs_theta += (convTwosComp(md.dx) / 4.95) / RADIUS ;
  float d_r = convTwosComp(md.dy) / 4.95;
  *total_x += d_r * sin(*abs_theta);
  *total_y += d_r * cos(*abs_theta);
  Serial.println("dr: " + String(d_r));
  Serial.println("d_theta: "+ String((convTwosComp(md.dx) / 4.95) / RADIUS));
  Serial.println("Angle: " + String(*abs_theta));
  Serial.println("Total y: " + String(*total_y));
  Serial.println("Total x: " + String(*total_x));
}
/////////////////////////////////////////////////////////////////
float error = 0; 
float error_prev = 0;
bool Rot_Ctrl = 0;//  Informed by server
int arrived = 0;
int turning_arrived = 0;
/////////////////////////////////////////////////////////////////
// PID Variables
float angular_error = 0;
float angular_error_prev = 0;
float pTerm;
float iTerm;
float dTerm;
float Kp = 2.5;
float Ki = 0;
float Kd = 0.05;
long currT = 0;
long prevT = 0;
float deltaT = 0;
float Kp_turning = 2;
float Ki_turning = 0.2;
float Kd_turning = 0.2;
int output;
float angle = PI/180 * 0;
/////////////////////////////////////////////////////////////////
float m = 0;
float c = 0;
/////////////////////////////////////////////////////////////////
//  y axis PID Variables
  float error_displacement = 0;
  // float pTerm_displacement = 0;
  // float iTerm_displacement = 0;
  // float dTerm_displacement = 0;

  float Kp_displacement = 2.5;
  float Ki_displacement = 0;
  float Kd_displacement = 0;
/////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////
  if(mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  }
  delay(3000);
}
void loop()
{  
  if(Rot_Ctrl && !turning_arrived)
  {
    //  Rotate
    OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
    error = (totalpath_x_int - RADIUS*angle);
    currT = micros();
    deltaT = ((float) (currT-prevT))/1.0e6;
    prevT = currT;
//    error = (abs(error) < 10) ? ( (error > 0) ? (15) : (-15) ) : (error);
//    error = (abs(totalpath_x_int - RADIUS*angle) < 5) ? (0) : (error);
    pTerm = error;
//    iTerm += (error * deltaT);
//    Serial.println("before shit");
//    dTerm = (error - error_prev)/deltaT;
//    Serial.println("avoid shit");
    output = Kp_turning * pTerm;
    error_prev = error;
    Serial.println("deltaT: " + String(deltaT));
    Serial.println("P: " + String(pTerm * Kp_turning));
    Serial.println("I: " + String(iTerm * Ki_turning));
    Serial.println("D: " + String(dTerm * Kd_turning));
    Serial.println("error: " + String(error));
    Serial.println("output: " + String(output));
    Serial.println("TOTAL_PATH_x: " + String(totalpath_x_int));
    Serial.println("");
    output = abs(output);
    if (error <= 0)
    {
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
    }
    else
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
    }
    output = abs(output);
    if (abs(error) < 3)
    {
      turning_arrived = 1; 
      output = 0; 
      Serial.println("Stopped");
    } 
    else 
    if (output > 255) 
    {
      output = 255;
    }
    else if (output < 38) 
    {
      output = 38;
    }
    analogWrite(PWMA, output);  //  TODO: See if mapping works
    analogWrite(PWMB, output);
  } else if (!arrived) {
    //  Rotate
    //OpticalFlowSensorReadings(md, &distance_x, &distance_y, &total_x1, &total_y1, &TOTAL_PATH_x, &TOTAL_PATH_y);
    OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
    currT = micros();
    deltaT = ((float) (currT-prevT))/1.0e6;
    prevT = currT;
    // controller

    OFS_Angular(md, &CURR_x, &CURR_y,  &ABSOLUTE_ANGLE);
    x_displacement(&CURR_x,&CURR_y,&A_x,&A_y,&B_x,&B_y,&angular_error);
    m = ((B_y - A_y) / (B_x - A_x));
    c = CURR_y - m*CURR_x;
    angular_error *= (CURR_y < m*CURR_x + c) ? (-1): (1);
    //angular_error = max(angular_error, (totalpath_x_int - RADIUS*angle));
    pTerm = (angular_error);
    iTerm += (angular_error*deltaT);
//    dTerm = (angular_error - angular_error_prev)/deltaT;
    output = pTerm * Kp + iTerm * Ki; //0.3 is good, 0.33 decent
    angular_error_prev = angular_error;
    Serial.println();
    Serial.println("Angular Error: " + String(angular_error));
    Serial.println("Output: " + String(output));
    //Serial.println("TOTAL_PATH_x: " + String(totalpath_x_int));
    //Serial.println("TOTAL_PATH_y: " + String(totalpath_y_int));
    //output = abs(output);
    {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CW  // ACW Rover
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CCW
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    error_displacement = B_y - totalpath_y_int;
    Bang_Constant = Kp_displacement * error_displacement;
    Bang_Constant = (Bang_Constant > 220) ? (220) : (Bang_Constant);
    Bang_Constant = (Bang_Constant < 30) ? (30) : (Bang_Constant);
    Serial.println(String(error_displacement) + "\t" + String(Bang_Constant));
    //////////////////////////////////////////////////////////////////////////////////////////////

    //output = abs(output);
    MotorSpeedA = Bang_Constant + output;
    MotorSpeedB = Bang_Constant - output;
    if (MotorSpeedA < 0) {MotorSpeedA = 0;}
    if (MotorSpeedA > 255) {MotorSpeedA = 255;}
    if (MotorSpeedB < 0) {MotorSpeedB = 0;}
    if (MotorSpeedB > 255) {MotorSpeedB = 255;}
    if ((totalpath_y_int - B_y < 10) && (totalpath_y_int - B_y > -10)) 
    {
        arrived = 1;
        MotorSpeedA = 0;
        MotorSpeedB = 0;
    }
    Serial.println("MotorSpeedA: " + String(MotorSpeedA));
    Serial.println("MotorSpeedB: " + String(MotorSpeedB));
    Serial.println("arrived" + String(arrived));
    analogWrite(PWMA, MotorSpeedA);  //  TODO: See if mapping works
    analogWrite(PWMB, MotorSpeedB);    
  }
}
