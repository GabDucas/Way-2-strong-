/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <Dynamixel2Arduino.h>

// Please modify it to suit your hardware.
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) // When using DynamixelShield
  #include <SoftwareSerial.h>
  SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
  #define DXL_SERIAL   Serial
  #define DEBUG_SERIAL soft_serial
  const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN
#elif defined(ARDUINO_SAM_DUE) // When using DynamixelShield
  #define DXL_SERIAL   Serial
  #define DEBUG_SERIAL SerialUSB
  const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN
#elif defined(ARDUINO_SAM_ZERO) // When using DynamixelShield
  #define DXL_SERIAL   Serial1
  #define DEBUG_SERIAL SerialUSB
  const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN
#elif defined(ARDUINO_OpenCM904) // When using official ROBOTIS board with DXL circuit.
  #define DXL_SERIAL   Serial3 //OpenCM9.04 EXP Board's DXL port Serial. (Serial1 for the DXL port on the OpenCM 9.04 board)
  #define DEBUG_SERIAL Serial
  const int DXL_DIR_PIN = 22; //OpenCM9.04 EXP Board's DIR PIN. (28 for the DXL port on the OpenCM 9.04 board)
#elif defined(ARDUINO_OpenCR) // When using official ROBOTIS board with DXL circuit.
  // For OpenCR, there is a DXL Power Enable pin, so you must initialize and control it.
  // Reference link : https://github.com/ROBOTIS-GIT/OpenCR/blob/master/arduino/opencr_arduino/opencr/libraries/DynamixelSDK/src/dynamixel_sdk/port_handler_arduino.cpp#L78
  #define DXL_SERIAL   Serial3
  #define DEBUG_SERIAL Serial
  const int DXL_DIR_PIN = 84; // OpenCR Board's DIR PIN.
#elif defined(ARDUINO_OpenRB)  // When using OpenRB-150
  //OpenRB does not require the DIR control pin.
  #define DXL_SERIAL Serial1
  #define DEBUG_SERIAL Serial
  const int DXL_DIR_PIN = -1;
#else // Other boards when using DynamixelShield
  #define DXL_SERIAL   Serial1
  #define DEBUG_SERIAL Serial
  const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN
#endif
 
//ÉPAULE
const uint8_t ID_EPAULE = 1;
const float DXL_PROTOCOL_VERSION = 2.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
//COUDE
const uint8_t ID_COUDE = 2;

//POIGNET
const uint8_t ID_POIGNET = 6;

//This namespace is required to use Control table item names
using namespace ControlTableItem;

  double theta_zero = 0.0;
  double theta_c = 0.0;
  double TORQUE = 0.0;

  double max_PWM_epaule = 0.0;
  double max_PWM_coude = 0.0;
  double max_PWM_poignet = 0.0;

void setup() {
  // put your setup code here, to run once:
  
  // Use UART port of DYNAMIXEL Shield to debug.
  DEBUG_SERIAL.begin(115200);
  while(!DEBUG_SERIAL);

//ÉPAULE
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(ID_EPAULE);

  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_EPAULE);
  // dxl.setOperatingMode(ID_EPAULE, OP_VELOCITY);
  dxl.setOperatingMode(ID_EPAULE, OP_EXTENDED_POSITION);
  // dxl.writeControlTableItem(CONTROL_MODE,ID_EPAULE, 0);
  // dxl.setOperatingMode(ID_EPAULE, OP_CURRENT);
  dxl.torqueOn(ID_EPAULE);
  

//COUDE
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_COUDE);
   dxl.setOperatingMode(ID_COUDE, OP_EXTENDED_POSITION);
  // dxl.setOperatingMode(ID_COUDE, OP_VELOCITY);
  // dxl.setOperatingMode(ID_COUDE, OP_POSITION);
  // dxl.setOperatingMode(ID_EPAULE, OP_CURRENT);
  dxl.torqueOn(ID_COUDE);

//POIGNET
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_POIGNET);
  dxl.setOperatingMode(ID_POIGNET, OP_VELOCITY);
  // dxl.setOperatingMode(ID_EPAULE, OP_CURRENT);
  dxl.torqueOn(ID_POIGNET);
//WIRE TABLE
  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 30);

  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 30);

  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 30);

//POUR METTRE LE 0 NE PAS DECOMMENTÉ
//ÉPAULE
  // delay(1000);
  // theta_zero = dxl.getPresentPosition(ID_EPAULE);
  // DEBUG_SERIAL.print(theta_zero);

  // dxl.torqueOff(ID_EPAULE);
  // dxl.writeControlTableItem(HOMING_OFFSET,ID_EPAULE,-560);
  // dxl.torqueOn(ID_EPAULE);
  calibration();
  anti_gravite();
}

void loop() {

  // DEBUG_SERIAL.print("THETA EPAULE: ");
  // DEBUG_SERIAL.print(dxl.getPresentPosition(ID_EPAULE, UNIT_DEGREE));
  // DEBUG_SERIAL.print(" THETA COUDE: ");
  // DEBUG_SERIAL.print(dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE));
  DEBUG_SERIAL.print(" PWM EPAULE ");
  DEBUG_SERIAL.print(dxl.getPresentPWM(ID_EPAULE));
  DEBUG_SERIAL.print(" PWM COUDE ");
  DEBUG_SERIAL.println(dxl.getPresentPWM(ID_COUDE));
 
  // DEBUG_SERIAL.print(" OFFSET : ");
  // DEBUG_SERIAL.println(OP_EXTENDED_POSITION);

  delay(1000);
}

void set_mode(int mode){
  //NICE TO HAVE:
  //PT FAUT TYPE CAST
  // if (dxl.readControlTableItem(OPERATING_MODE,ID_EPAULE) == mode){
  //   return;
  // }
  // else{
  dxl.torqueOff(ID_COUDE);
  dxl.setOperatingMode(ID_COUDE, mode);
  dxl.torqueOn(ID_COUDE);

  dxl.torqueOff(ID_EPAULE);
  dxl.setOperatingMode(ID_EPAULE, mode);
  dxl.torqueOn(ID_EPAULE);

  dxl.torqueOff(ID_POIGNET);
  dxl.setOperatingMode(ID_POIGNET, mode);
  dxl.torqueOn(ID_POIGNET);
}

void calibration(){
  float PWM_epaule = 0.0;
  float PWM_coude = 0.0;
  
  set_mode(OP_EXTENDED_POSITION);

  dxl.setGoalPosition(ID_EPAULE, -97, UNIT_DEGREE);//VALEUR POUR 90 deg 
  dxl.setGoalPosition(ID_COUDE, 0, UNIT_DEGREE);//VALEUR POUR 90 deg 
  delay(1000);

  for (int i = 0; i<10 ; i++)
  {
    PWM_epaule += dxl.getPresentPWM(ID_EPAULE);
    PWM_coude += dxl.getPresentPWM(ID_COUDE);
    DEBUG_SERIAL.println(dxl.getPresentPWM(ID_EPAULE));
    delay(200);
  }
  max_PWM_epaule = abs(PWM_epaule/10);
  max_PWM_coude = abs(PWM_coude/10);
}

void anti_gravite(){
  set_mode(OP_VELOCITY);

  dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule+5);
  dxl.setGoalVelocity(ID_EPAULE, 0);

  dxl.setGoalPWM(ID_COUDE, max_PWM_coude+5);
  dxl.setGoalVelocity(ID_COUDE, 0);
}
