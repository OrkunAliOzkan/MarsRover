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

/////////////////////////////////////////////////////////////////
//  obstacle avoidance

//  camera readings
  void camera_readings(float *camera_readings_type, float *camera_readings_displacemet, float *camera_readings_displacemet) {
*type = 7;
*camera_readings_displacemet = 50;
*camera_readings_displacemet = 0;
}

int sign(float number){
    return (number >= 0) ? (1) : (-1);
}

//  radius of an A2 base at its widest
#define MAXIMUM_HOME_RADIUS                 174.75
//  radius of a ping pong ball
#define BALL_RADIUS                         20
//  minimum boundary for object to to from rover to not need to avoid
#define MINIMUM_SAFE_OBJECT_X_DISPLACEMENT  70
//  rover width
#define ROVER_WIDTH                         200

// type, displacement, angle 
float camera_readings_type = 0;
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

/////////////////////////////////////////////////////////////////

//  state mashine parameters for the drive process (stop turn go)
int turning_complete = 1;
int straight_line_complete = 1;

long lastCycle = 0;
void setup()
{
    Serial.begin(115200);
    Wire.begin();
    //  SPI Initialisation
    SPI.begin();
    //sets SPI clock to 1/32 of the ESP32's clock
    SPI.setClockDivider(SPI_CLOCK_DIV32); 
    //in SPI mode 3, data is sampled on the 
    //falling edge and shifted out on the rising edge
    SPI.setDataMode(SPI_MODE3); 
    SPI.setBitOrder(MSBFIRST);

    turning_complete = 0;
    straight_line_complete = 0;
}

void loop()
{
  //  camera readings
    camera_readings(&camera_readings_type, &camera_readings_displacemet, &camera_readings_angle);

  //  update values
    object_x = current_x + (camera_readings_displacemet*sin( (PI * camera_readings_angle) / 180 ));
    object_y = current_y + (camera_readings_displacemet*cos( (PI * camera_readings_angle) / 180 ));

    object_radius = (camera_readings_type == 7) ? (MAXIMUM_HOME_RADIUS) : (BALL_RADIUS);

    object_rover_x_difference = abs(object_x) + object_radius - (ROVER_WIDTH / 2) - current_x;

    object_displacement = sqrt(pow((object_x - current_x), 2) + pow((object_y - current_y), 2));
    object_angle = atan2(object_y - current_y, object_x - current_x);

    //  if there is an object and it is less than 10cm away
    if(  
        (camera_readings_type) &&
        (object_displacement < 100) &&
        (object_rover_x_difference < MINIMUM_SAFE_OBJECT_X_DISPLACEMENT) && 
        (!emergancy_corner_count)){
      //  if haven't already emergancy breaked
        //  stop
        emergancy_breaked = 1;

        //  stash desired destination coodinate
        camera_stashed_x = B_x;
        camera_stashed_y = B_y;

        //  tell rover to move left or right in opposite direction to the object
        //  mximum amount needed to move out by eye would probably be 2 of the object displacements.
        B_x = (object_angle >  0) ? (-2*object_radius) : (2*object_radius); 

        //  initialise the emergancy procedure
            emergancy_corner_count++;
    }
    //  0)
    if(emergancy_corner_count){
          //  stop
            analogWrite(PWMA, 0); 
            analogWrite(PWMB, 0);

            if((turning_complete) && (straight_line_complete)){
            switch(emergancy_corner_count)
            {
                case 1:{
                //  tell rover to move forward or backward in direction to the object
                //  mximum amount needed to move out by eye would probably be 2 of the object displacements.
                B_y = (sign(current_angle) * (2*object_radius));
                }

                case 2:{
                //  return to original x position
                B_x = camera_stashed_x;
                }

                case 3:{
                //  return to travelling on original path
                B_y = camera_stashed_y;
                emergancy_corner_count = 0;
                }
            }
            
            default:{
                Serial.println("Should never be here.\n");
              //  stop
                analogWrite(PWMA, 0); 
                analogWrite(PWMB, 0);
            }
            }
            turning_complete = 0;
            straight_line_complete = 0;
    }

    if (!turning_complete) {
        turning_complete = 1;
        differential_PWM_output = 0; 
        // Debugging Messages
        Serial.println("Rover is turning");
        Serial.println("target_angle: " + String(target_angle));
        Serial.println("angular error: " + String(angular_error));
    } 
    else if (!straight_line_complete) {
        // Straight Line Logic
        straight_line_complete = 1;

        prev_x = B_x;
        prev_y = B_y;

        tcp_send = "---\n";
        tcp_send += ("Straight Line Complete\n");
    }

    delay(1000);
}
