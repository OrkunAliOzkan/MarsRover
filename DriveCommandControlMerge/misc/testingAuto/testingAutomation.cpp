///////////////////////////////////////////////
#include <string>
#include <vector>
#include <cmath>
#define PI 3.141592653589793
///////////////////////////////////////////////
//  Drive parameters
float A_x = 0; // TODO: Define @ end of travel
float A_y = 0; // TODO: Define @ end of travel

float B_x = 0;
float B_y = 750;

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
int counter = 0;
///////////////////////////////////////////////
//  Declaration
void automation(
    bool isTurning, bool isTravelling,
    bool * turningLeft, bool * turningPrevious, int * counter,
    float arena_width, float arena_height,
    int side_sections_spans, int mid_sections_spans,
    float x_pos, float y_pos,
    float * x_des, float * y_des, float * bearing
);
///////////////////////////////////////////////
int main(void)
{
    while(1)
    {
        ///////////////////////////////////////////
        //  Readings
        if ((turning_arrived && arrived) || 
            (UpdateRead))
        {
            /*
                //  TODO: Make parameters work
            automation(
            isTurning, isTravelling,
            turningLeft, turningPrevious, counter,
            arena_width, arena_height,
            side_sections_spans, mid_sections_spans,
            x_pos, y_pos,
            x_des, y_des, bearing
            );
            */



            //read_cartesian = code_body.HTTPGET();
            angle = read_cartesian[0] / 180 * PI;
            B_y = read_cartesian[1];
            displaceBy = sqrt(
                            pow((B_y - A_y), 2) + 
                            pow((B_x - A_x), 2)
                            );
            turning_arrived = 0;
            arrived = 0;
            UpdateRead = 0;
        }
        ///////////////////////////////////////////
        //  Movement
        if(!turning_arrived)
        {
            //  Rotate
                turning_arrived = 1; 
    
                A_x = CURR_x;
                A_y = CURR_y;
    
                UpdateRead = 1;
        } 
        else if (!arrived && turning_arrived) {
            //  Straight
                arrived = 1;
                UpdateRead = 1;
        }
    }
}
///////////////////////////////////////////////
//  Implementation
void automation(
    bool isTurning, bool isTravelling,
    bool * turningLeft, bool * turningPrevious, int * counter,
    float arena_width, float arena_height,
    int side_sections_spans, int mid_sections_spans,
    float x_pos, float y_pos,
    float * x_des, float * y_des, float * bearing

)
{
    if(isTurning) {
    /*
        Characterised desired motion:
        N/A, R, N/A, L, N/A, L, N/A, R
        Repeat
    */
        int sign = (*turningLeft) ? (1) : (-1);
        *bearing = sign * 90;
        *turningPrevious = *turningLeft;
        *turningLeft = !(*turningLeft && *turningPrevious);
    }
    else if(isTravelling && !isTurning){
        //  Determining x
        /////////////////////////////////////////////////////////////////////////
            float DisplaceByX = 0;
            DisplaceByX = (int)(*counter % 2 == 0) * (arena_width);
            DisplaceByX = (int)(*counter % 4 == 0) * (-2 * arena_width);
            *x_des = x_pos + DisplaceByX;
        /////////////////////////////////////////////////////////////////////////
        //  Determining y
            float DisplaceByY = 0;
            float scalingFactor = 0;
            scalingFactor = (
                    (*counter < 2*side_sections_spans) || 
                    (*counter > 2 * (mid_sections_spans + side_sections_spans))) ? 
                    (side_sections_spans) : (mid_sections_spans);

            DisplaceByY = (int)(*counter % 2 != 0) * (arena_height / (2*scalingFactor));
            *y_des = y_pos + DisplaceByY;
        /////////////////////////////////////////////////////////////////////////
        *counter++;
    }
}