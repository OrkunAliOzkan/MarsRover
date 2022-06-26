// // GOOD

//     if (!turning_complete) {
//         // Rotation Logic

//         OFS_Readings
//                 (            
//                 md, 
//                 &total_path_x_R, 
//                 &total_path_y_R, 
//                 &x_coordinate_R, 
//                 &y_coordinate_R,
//                 &angle_R
//                 );

//         offset_error = total_path_y_R;
//         Serial.println("offset_error:\t" + String(offset_error));
//         offset_error = total_path_y_R;

//         // Serial.println("in !turning_complete");
//         // simplistic dead reckoning
//         //current_angle = ((float) totalpath_x_int) / RADIUS + prev_angle;
//         if (abs(rotation_controller_p*angle_R - target_angle) < 0.05) {
//             Serial.println("in error good");
//             // brake
//             analogWrite(PWMA, 0); 
//             analogWrite(PWMB, 0);

//             total_path_x_R = 0;
//             total_path_y_R = 0;
//             current_angle = rotation_controller_p*angle_R;    //  Set the current angle
//             turning_complete = 1;
//             differential_PWM_output = 0; 
//             offset_PWM_output = 0; 
            
//             // resetting PID variables
//             prevT = 0;
//             p_term_angle = 0;
//             i_term_angle = 0;
//             d_term_angle = 0;
//             // resetting OFS_Cartesian variables
//             prescaled_tx = 0;
//   //            prescaled_ty = 0;
//             totalpath_x_int = 0;
            
//             // simplistic dead reckoning
//             //prev_angle = current_angle;
//         } else {
//         Serial.println("in else");
//             // turning not complete
//             currT = micros();
//             deltaT = ((float) (currT-prevT))/1.0e6;
            
//             p_term_angle = offset_error;
//             i_term_angle += offset_error * deltaT;
//             // d_term_angle = (offset_error - offset_error_prev)/deltaT;

//             offset_PWM_output = abs(Kp_rotation * p_term_angle + Ki_rotation * i_term_angle);

//             // guards to keep output within bounds

//             // set the right motor directions
//             if (target_angle > 0) {
//                 digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
//                 digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
//             } else {
//                 digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
//                 digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
//             }

//             // power the motors
//             analogWrite(PWMA, MIN_ROT_PWM - offset_PWM_output);  
//             analogWrite(PWMB, MIN_ROT_PWM + offset_PWM_output);

//             // update variables for next cycle
//             prevT = currT;
//             offset_error_prev = offset_error;

//             Serial.println("total_path_x_R:\t"  + String(total_path_x_R));
//             Serial.println("total_path_y_R:\t"  + String(total_path_y_R));
//             Serial.println("x_coordinate_R:\t"  + String(x_coordinate_R));
//             Serial.println("y_coordinate_R:\t"  + String(y_coordinate_R));
//             Serial.println("angle_R:\t"         + String(angle_R)       );
//         }
//         delay(30);
//     }

// // BAD

// /*
//     double offset_error;

//     double p_term_offset;
//     double i_term_offset;
//     double d_term_offset;

//     double Kp_offset = 1;
//     double Kd_offset = 1;

//     double rotation_deviation_error;
//     double rotation_deviation_error_prev;

//     double offset_PWM_output;
// */

//     if (!turning_complete) {
//         // Rotation Logic
//         OFS_Readings
//             (            
//             md, 
//             & total_path_x, 
//             & total_path_y, 
//             & current_x, 
//             & current_y,
//             & current_angle
//             );

//         angular_error = (current_angle - target_angle);
//         rotation_deviation_error = total_path_y;

//         if (abs(angular_error) < 0.5) {
//             // brake
//             analogWrite(PWMA, 0); 
//             analogWrite(PWMB, 0);

//             turning_complete = 1;
//             differential_PWM_output = 0; 
            
//             // resetting angle PID variables
//             prevT = 0;
//             p_term_angle = 0;
//             i_term_angle = 0;
//             d_term_angle = 0;

//             // resetting offset PID variables
//             p_term_offset = 0;
//             i_term_offset = 0;
//             d_term_offset = 0;
            
//             // resetting OFS readings
//             total_path_x = 0;
//             total_path_y = 0;
            
//             // tcp_send = "---\n";
//             // tcp_send += ("Turning Complete\n");
//         } else {
//             // turning not complete
//             currT = micros();
//             deltaT = ((double) (currT-prevT))/1.0e6;
            
//             //  control angle
//             p_term_angle = angular_error;
//             i_term_angle += angular_error * deltaT;
//             d_term_angle = (angular_error - angular_error_prev)/deltaT;

//             differential_PWM_output = abs(Kp_rotation * p_term_angle + Ki_rotation * i_term_angle);

//             //  control offset from centre
//             p_term_deviation = rotation_deviation_error;
//             i_term_deviation += rotation_deviation_error * deltaT;
//             d_term__deviation = (rotation_deviation_error - rotation_deviation_error_prev)/deltaT;
//             deviation_PWM_output = abs(Kp_deviation * p_term_deviation + Ki_deviation * i_term_deviation);

//             // guards to keep output within bounds
//             if (differential_PWM_output > MAX_ROTATION_PWM) {
//                 differential_PWM_output = MAX_ROTATION_PWM;
//             }
//             else if (differential_PWM_output < MIN_ROTATION_PWM) {
//                 differential_PWM_output = MIN_ROTATION_PWM;
//             }

//             // if you want to go left
//             if (angular_error <= 0) {
//                 digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); //LW_CW  // ACW Rover
//                 digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); //RW_CW
//             } else {
//                 digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); //LW_CCW  // CW Rover
//                 digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); //RW_CCW
//             }

//             // power the motors
//             analogWrite(PWMA, differential_PWM_output + deviation_PWM_output);  
//             analogWrite(PWMB, differential_PWM_output - deviation_PWM_output);

//             // update variables for next cycle
//             prevT = currT;
//             angular_error_prev = angular_error;
//             rotation_deviation_error_prev = rotation_deviation_error;
//         }

//         // Debugging Messages
//         Serial.println("Rover is turning");
//         Serial.println("target_angle: " + String(target_angle));
//         Serial.println("angular error: " + String(angular_error));
//         Serial.println("TOTAL_PATH_x: " + String(total_path_x));
//         Serial.println("TOTAL_PATH_y: " + String(total_path_y));
//     } 