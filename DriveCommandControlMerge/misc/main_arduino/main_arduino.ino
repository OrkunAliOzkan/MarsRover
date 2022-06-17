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

WiFiClient client; // Use WiFiClient class to create TCP connections
const uint16_t port = 4001;
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
//  automation
  void automation(
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

    //mode_ = "A";

    if (mode_ == "M") {
      // update target angle
      updateTargets(&B_x, &B_y, &current_x, &current_y, &current_angle, &target_displacement, &target_angle);
        
    }
    else if (mode_ == "A") {
      Serial.println("Shouldn't be seeing this in loop!\n");

      // update position to travel to
      /*
      automation(
      &counter,
      arena_width, arena_height,
      side_sections_spans,  mid_sections_spans,
      x_pos, y_pos,
      &x_des, &y_des, &target_bearing
      );  
      */
    }


    Serial.println("cx: " + String(current_x));
    Serial.println("cy: " + String(current_y));
    Serial.println("bx: " + String(B_x));
    Serial.println("by: " + String(B_y));
    Serial.println("current angle: " + String(current_angle));   
    Serial.println("target displacement: " + String(target_displacement));
    Serial.println("target angle: " + String(target_angle));

    turning_complete = 0;
    straight_line_complete = 0;
}

String location_info = "";

void loop()
{
    Serial.println(micros() - lastCycle);
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
    Serial.println(tcp_send);
    tcp_send = "";

    // // Periodically send data back to server
    if (millis() - last_TCP_post > TCP_post_period) {
        // Serial.println(tcp_send);
        location_info = "{x: " + String(current_x) + " y: " + String(current_y) + " angle: " + String(current_angle) + "}";
        client.print(location_info);
        last_TCP_post = millis();
    }

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
        
        
        Serial.println("//////////////////////////////////////////////////////////////////");

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
    else if((mode_ == "A") && (0)){
      Serial.println("Shouldn't be seeing this in loop!\n");
    }

    if (!turning_complete) {
        // Rotation Logic
        OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        angular_error = (totalpath_x_int - RADIUS*target_angle);
        // simplistic dead reckoning
        current_angle = ((float) totalpath_x_int) / RADIUS + prev_angle;
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
            
            // simplistic dead reckoning
            prev_angle = current_angle;

            tcp_send = "---\n";
            tcp_send += ("Turning Complete\n");
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

    } 
    else if (!straight_line_complete) {
        // Straight Line Logic
        OFS_Cartesian(md, &prescaled_tx, &prescaled_ty, &totalpath_x_int, &totalpath_y_int);
        displacement_error = target_displacement - totalpath_y_int;
        // simplistic dead reckoning
        current_x = prev_x + totalpath_y_int * cos(current_angle);
        current_y = prev_y + totalpath_y_int * sin(current_angle);
        
        if (abs(displacement_error) < 10) {
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

            tcp_send = "---\n";
            tcp_send += ("Straight Line Complete\n");
        } else {
            //  y-axis pid controller
            displacement_PWM_output = Kp_displacement * displacement_error;
            Serial.println("displacement_error:\t" + String(displacement_error));
            //  guards
            displacement_PWM_output = (displacement_PWM_output > MAX_PWM) ? (MAX_PWM) : (displacement_PWM_output);
            displacement_PWM_output = (displacement_PWM_output < MIN_PWM) ?  (MIN_PWM)  : (displacement_PWM_output);
            //  strictly for testing purposes FIXME: DELETE
            MotorSpeedA = displacement_PWM_output;
            MotorSpeedB = displacement_PWM_output;

//            // deviation pid controller
//            currT = micros();
//            deltaT = ((float) (currT-prevT))/1.0e6;
//
//            angular_error = (totalpath_x_int);
//
//            p_term_angle = (angular_error);
//            i_term_angle += (angular_error*deltaT);
//            //    d_term_angle = (angular_error - angular_error_prev)/deltaT;
//            differential_PWM_output = p_term_angle * Kp_deviation + i_term_angle * Ki_deviation; //0.3 is good, 0.33 decent
//
//            MotorSpeedA = displacement_PWM_output + differential_PWM_output;
//            MotorSpeedB = displacement_PWM_output - differential_PWM_output;

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
    /*
        tcp_send +="---\n";
        tcp_send +=("Rover moving in straight line:");
        tcp_send += "\n";
        tcp_send +=("Angular Error: " + String(angular_error));
        tcp_send += "\n";
        tcp_send +=("Displacement Error: " + String(displacement_error));
        tcp_send += "\n";
        tcp_send +=("differential_PWM_output: " + String(differential_PWM_output));
        tcp_send += "\n";
        tcp_send +=("TOTAL_PATH_x: " + String(totalpath_x_int));
        tcp_send += "\n";
        tcp_send +=("TOTAL_PATH_y: " + String(totalpath_y_int));
        tcp_send += "\n";
        tcp_send +=("MotorSpeedA: " + String(MotorSpeedA));
        tcp_send += "\n";
        tcp_send +=("MotorSpeedB: " + String(MotorSpeedB));
        tcp_send += "\n";
    */
    }
}
