///////////////////////////////////////////////
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define PI 3.141592653589793
///////////////////////////////////////////////
//  Drive parameters
float A_x = 0; // TODO: Define @ end of travel
float A_y = 0; // TODO: Define @ end of travel

float B_x = 0;
float B_y = 0;

float displaceBy = 0;
float ABSOLUTE_ANGLE = 0;

float CURR_x = 0;
float CURR_y = 75;
float angle = 0;
///////////////////////////////////////////////
int arrived = 0;
int turning_arrived = 0;
///////////////////////////////////////////////
std::string post_data = "";
std::vector<float> read_cartesian;
///////////////////////////////////////////////
bool UpdateRead = 0;
///////////////////////////////////////////////
int automation_counter = 0;
///////////////////////////////////////////////
//  Declaration
void automation(
    int * counter,
    float arena_width, float arena_height,
    int side_sections_spans, int mid_sections_spans,
    float x_pos, float y_pos,
    float * x_des, float * y_des, float * bearing
);
///////////////////////////////////////////////
bool turningLeft = 0;
bool turningPrevious = 0;
///////////////////////////////////////////////
bool automationOn = 1;
///////////////////////////////////////////////
int main(void)
{
    while(B_y < 1000)
    {
        ///////////////////////////////////////////
        //  Readings
        if (turning_arrived && arrived)
        {
            turning_arrived = 0;
            arrived = 0;
            UpdateRead = 0;
            
            A_x = B_x;
            A_y = B_y;
            
            if(automationOn)
            {
                automation(
                &automation_counter,
                232, 1050,
                3, 2,
                A_x, A_y,
                &B_x, &B_y, &angle
                );
            }
            
            
            std::cout   << "\n---------------------\n"
                        << "automation_counter:\t" << automation_counter
                        << "B_x:\t" << B_x << "\t" 
                        << "B_y:\t" << B_y << "\n"
                        << "angle:\t" << angle;

            displaceBy = sqrt(
                            pow((B_y - A_y), 2) + 
                            pow((B_x - A_x), 2)
                            );
        }
        ///////////////////////////////////////////
        //  Movement
        if(!turning_arrived)
        {
            //  Rotate
                turning_arrived = 1; 
    
                UpdateRead = 1;
        } 
        else if (!arrived && turning_arrived) {
            //  Straight
                arrived = 1;
                UpdateRead = 1;
                //A_x = B_x;
                //A_y = B_y;
        }
    }
}
///////////////////////////////////////////////
//  Implementation
void automation(
    int * counter,
    float arena_width, float arena_height,
    int side_sections_spans, int mid_sections_spans,
    float x_pos, float y_pos,
    float * x_des, float * y_des, float * bearing
)
{
    {
        int sign = ((*counter % 4 == 0) || (*counter % 4 == 1)) 
                    ? (1) : (-1);
        *bearing = sign * 90 * (*counter != 0);
    }
    {
        //  Determining x
        /////////////////////////////////////////////////////////////////////////
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
        /////////////////////////////////////////////////////////////////////////
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
        /////////////////////////////////////////////////////////////////////////
        }
        *counter += 1;
    }
}