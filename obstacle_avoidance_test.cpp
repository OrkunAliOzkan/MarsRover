#include <vector>
#include <string>

std::vector<std::string> states = { "_2000", "+600", "+2000", "-600", "-2000", "+600", "+2000", "+1800" };
int closest, current_max;
std::string H1, H2;
int state_direction;
int avoidance_timer = 0; //prolly not int
int ball_distance[];
int building_distance, building_distance1, building_distance2;

int closest_obj_distance () {
    current_max = 0;
    for (int i=0; i<6; i++) {
        if (ball_distance[i] > current_max) {current_max = ball_distance[i];}
    }
    if (building_distance > current_max) {current_max = building_distance;}
    if (building_distance1 > current_max) {current_max = building_distance1;}
    if (building_distance2 > current_max) {current_max = building_distance2;}
    if (current_max == 0) {return 0;}
    else {return current_max;}
}

std::vector<std::string> insert_avoid_states (std::vector<std::string> states) {
    switch (state_direction) {
        case 0: {
            H1 = "+400";
            H2 = "-10000";
        };
        break;
        case 1: {
            H1 = "-400";
            H2 = "+10000";
        };
        break;
        case 2: {
            H1 = "-400";
            H2 = "+10000";
        };
        break;
        case 3: {
            H1 = "+400";
            H2 = "-10000";
        };
        break;
    }
    states.insert((states.begin() + 1), H1);
    states.insert((states.begin() + 2), H2);
}

void obstacle_avoidance () {
    if (avoidance_timer > 5) { //seconds
        closest = closest_obj_distance();
        if (closest != 0) {
            if (closest < 500) {
                states = insert_avoid_states(states);
                //start avoidance timer here
            }
        }
    }
}

void change_state () {
    states.erase(states.begin());
}


//call change_state() whenever the motion stop flag or whatever is set to high (like call it on the next line)

//call obstacle_avoidance()