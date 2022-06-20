// PIO includes
  #include <SPI.h>
  #include <Wire.h>

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

// Current position and angle
  float current_x = 100;
  float current_y = 100;
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
  float target_angle = 2*PI;

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
        Serial.println("Mouse cam failed to init");
        while(1);
    }
    turning_complete = 0;
}

void loop()
{
    if (!turning_complete) {
        // Rotation Logic
        OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        angular_error = (totalpath_x_int - RADIUS*target_angle);
        // simplistic dead reckoning
        current_angle = ((float) totalpath_x_int) / RADIUS + prev_angle;
        if (abs(angular_error) < 0.5) {
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
            
            // simplistic dead reckoning
            prev_angle = current_angle;

            Serial.println("---\n");
            Serial.println("Turning Complete\n");
        } else {
            // turning not complete
            currT = micros();
            deltaT = ((float) (currT-prevT))/1.0e6;
            
            p_term_angle = angular_error;
            i_term_angle += angular_error * deltaT;
            d_term_angle = (angular_error - angular_error_prev)/deltaT;

            differential_PWM_output = abs(Kp_rotation * p_term_angle + Ki_rotation * i_term_angle);

            // guards to keep output within bounds
            if (differential_PWM_output > 255) {
                differential_PWM_output = 255;
            }
            else if (differential_PWM_output < MIN_PWM) {
                differential_PWM_output = MIN_PWM;
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
        Serial.println("Rover is turning");
        Serial.println("target_angle: " + String(target_angle));
        Serial.println("angular error: " + String(angular_error));
        /*
        tcp_send += "---\n";
        tcp_send +=("Rover is turning");
        tcp_send += "\n";
        tcp_send +=("target_angle: " + String(target_angle));
        tcp_send += "\n";
        tcp_send +=("angular error: " + String(angular_error));
        tcp_send += "\n";
        tcp_send +=("deltaT: " + String(deltaT));
        tcp_send += "\n";
        tcp_send +=("P: " + String(p_term_angle * Kp_rotation));
        tcp_send += "\n";
        tcp_send +=("I: " + String(i_term_angle * Ki_rotation));
        tcp_send += "\n";
        tcp_send +=("D: " + String(d_term_angle * Kd_rotation));
        tcp_send += "\n";
        tcp_send +=("differential_PWM_output: " + String(differential_PWM_output));
        tcp_send += "\n";
        tcp_send +=("current angle: " + String(current_angle));
        tcp_send += "\n";
        tcp_send +=("TOTAL_PATH_x: " + String(totalpath_x_int));
        tcp_send += "\n";
        tcp_send +=("TOTAL_PATH_y: " + String(totalpath_y_int));
        tcp_send += "\n";
        */

    } 
}
