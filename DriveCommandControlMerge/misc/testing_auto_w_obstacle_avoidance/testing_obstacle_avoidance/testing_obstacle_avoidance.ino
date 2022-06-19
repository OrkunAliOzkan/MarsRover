// PIO includes
  #include <SPI.h>
  #include <Wire.h>
  #include <WiFi.h>

/////////////////////////////////////////////////////////////////

// Starting coordinates
float A_x = 0; // TODO: Define at end of movement
float A_y = 0; // TODO: Define at end of movement

// Destination coordinates
float B_x = 0;
float B_y = 750;

// Current position and angle
float current_x = A_x;
float current_y = A_y;
float current_angle = PI/2;

/////////////////////////////////////////////////////////////////
//  obstacle avoidance

//  camera readings
void camera_readings(int *camera_readings_type, float *camera_readings_displacemet, float *camera_readings_angle) {
*camera_readings_type = 7;
*camera_readings_displacemet = 40;
*camera_readings_angle = 0;
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

    Serial.println("Init\n");
}

void loop()
{
  //  camera readings
    camera_readings(&camera_readings_type, &camera_readings_displacemet, &camera_readings_angle);

  //  update values
    object_x = current_x + (camera_readings_displacemet*sin( (PI * camera_readings_angle) / 180 ));
    object_y = current_y + (camera_readings_displacemet*cos( (PI * camera_readings_angle) / 180 ));

    object_radius = (camera_readings_type == 7) ? (MAXIMUM_HOME_RADIUS) : (BALL_RADIUS);

    object_rover_x_difference = abs(object_x - current_x) /*+ object_radius - (ROVER_WIDTH / 2)*/;

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

        //  stash desired destination coodinate
        camera_stashed_x = B_x;
        camera_stashed_y = B_y;

        //  tell rover to move left or right in opposite direction to the object
        //  mximum amount needed to move out by eye would probably be 2 of the object displacements.
        B_x = (object_angle >  0) ? (-2*object_radius) : (2*object_radius); 
        B_y = current_y;

    Serial.println(
    "\nemergancy_corner_count:\t" + String(emergancy_corner_count) +
    "\nB_x:\t" + String(B_x) +
    "\nB_y:\t" + String(B_y) +
    "\nobject_angle:\t" + String(object_angle)
    
    );

        emergancy_corner_count = 1;
        //  initialise the emergancy procedure
    }
    //  0)
    else if((emergancy_corner_count > 0) && (!avoided)){
            if((turning_complete) && (straight_line_complete)){
            switch(emergancy_corner_count)
            {
                case 1:{
                Serial.println("1\n");
                //  tell rover to move forward or backward in direction to the object
                //  mximum amount needed to move out by eye would probably be 2 of the object displacements.
                B_y = (sign(current_angle) * (2*object_radius));
//                Serial.println("B_yyy:\t" + String(B_y));
//                Serial.println("sign(current_angle):\t" + String(sign(current_angle)));
//                Serial.println("object_radius:\t" + String(object_radius));
                break;
                }

                case 2:{
                Serial.println("2\n");
                //  return to original x position
                B_x = camera_stashed_x;
                break;

                }

                case 3:{
                Serial.println("3\n");
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

    if (!turning_complete) {
        turning_complete = 1;
    } 
    else if (!straight_line_complete) {
        // Straight Line Logic
        straight_line_complete = 1;

        A_x = B_x;
        A_y = B_y;

        current_x = A_x;
        current_y = A_y;

        Serial.println("---\n");
        Serial.println("Straight Line Complete\n");
            Serial.println(
            "end\nemergancy_corner_count:\t" + String(emergancy_corner_count) +
            "\nB_x:\t" + String(B_x) +
            "\nB_y:\t" + String(B_y));
    }


    Serial.println("\n");
    delay(1000);
}
