#include <Robojax_L298N_DC_motor.h>
#include <cmath>
#include <iostream>
#include <Arduino.h>
// motor 1 settings
#define CHA 0
#define PWMA 17 // PWMA // this pin must be PWM enabled pin if Arduino board is used
#define AIN1 14 // AIN1
#define AIN2 16 //AIN2
// motor 2 settings
#define BIN1 4 //BIN1
#define BIN2 15 //BIN2
#define PWMB 2 // PWMB // this pin must be PWM enabled pin if Arduino board is used
#define CHB 1

const int CCW = 2; // do not change
const int CW  = 1; // do not change
double motor1_calibration = 1;
double motor2_calibration = 0.9;
double motor1_strength = 0;
double motor2_strength = 0;
#define motor1 1 // left
#define motor2 2 // right
char key = ' ';
//Robojax_L298N_DC_motor robot(AIN1, AIN2, PWMA, CHA,  BIN1, BIN2, PWMB, CHB);
// for two motors with debug information
Robojax_L298N_DC_motor robot(AIN1, AIN2, PWMA, CHA, BIN1, BIN2, PWMB, CHB, true);
void setup() {
  Serial.begin(115200);
  robot.begin();
}
void loop() {

  motor1_strength = 70 * motor1_calibration;
  motor2_strength = 70 * motor2_calibration;
  Serial.println(motor1_strength);
  Serial.println(round(motor1_strength));
  Serial.println(motor2_strength);
  if (Serial.available()) {
    key = Serial.read();
    Serial.flush();
    robot.rotate(motor1, motor1_strength, CW);
    robot.rotate(motor2, motor2_strength, CCW);
    delay(100);
    while (Serial.available()) {
      key = Serial.read();
    delay(10);
    }
  }
  else {
   robot.rotate(motor1, 0, CW);
   robot.rotate(motor2, 0, CCW); 
  }
//   for(int i=0; i<=100; i++)
//   {
//     robot.rotate(motor1, i, CW);// turn motor1 with i% speed in CW direction (whatever is i)
//     robot.rotate(motor2, i, CCW);// turn motor1 with i% speed in CW direction (whatever is i) 
//     delay(100);
//   }
//   delay(2000);
//   robot.brake(1);
//   robot.brake(2);
//   delay(2000);  
  
  }
