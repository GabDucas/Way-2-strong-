#include <Dynamixel2Arduino.h>

// PARAMS SPÉCIFIQUE À OPENCR
#define DXL_SERIAL   Serial3
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 84; // OpenCR Board's DIR PIN.
 
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

double max_PWM_epaule = 0.0;
double max_PWM_coude = 0.0;
double max_PWM_poignet = 0.0;

float zero_offset_epaule = -5;
float zero_offset_coude = -3;
float zero_offset_poignet = 355.62;

void setup() {
  // Use UART port of DYNAMIXEL Shield to debug.
  DEBUG_SERIAL.begin(115200);
  while(!DEBUG_SERIAL);

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(ID_EPAULE);//TODO: BESOIN DE CA??
//ÉPAULE
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_EPAULE);
  dxl.setOperatingMode(ID_EPAULE, OP_EXTENDED_POSITION);

//COUDE
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_COUDE);
  dxl.setOperatingMode(ID_COUDE, OP_EXTENDED_POSITION);

//POIGNET
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(ID_POIGNET);
  dxl.setOperatingMode(ID_POIGNET, OP_EXTENDED_POSITION);

//WIRE TABLE
  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 30);
// TODO: À TESTER:
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_EPAULE, 5);// TODO: VOIR SI CA MARCHE LOL
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_COUDE, 5);
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_POIGNET, 5);

  // dxl.torqueOn(ID_EPAULE);
  // dxl.torqueOn(ID_COUDE);
  // dxl.torqueOn(ID_POIGNET);

  // calibration();
  // anti_gravite();

//POUR METTRE LE 0 NE PAS DECOMMENTÉ
//ÉPAULE
// double theta_zero = 0.0;
//   delay(1000);
//   theta_zero = dxl.getPresentPosition(ID_EPAULE);
//   DEBUG_SERIAL.print(theta_zero);

//   dxl.torqueOff(ID_EPAULE);
//   dxl.writeControlTableItem(HOMING_OFFSET,ID_EPAULE,-384);
//   dxl.torqueOn(ID_EPAULE);

//COUDE
// double theta_zero_coude = 0.0;
//   delay(1000);
//   theta_zero_coude = dxl.getPresentPosition(ID_COUDE);
//   DEBUG_SERIAL.print(theta_zero_coude);

//   dxl.torqueOff(ID_COUDE);
//   dxl.writeControlTableItem(HOMING_OFFSET,ID_COUDE,-327);
//   dxl.torqueOn(ID_COUDE);

//Poignet
// double theta_zero_poignet = 0.0;
//   delay(1000);
//   theta_zero_poignet = dxl.getPresentPosition(ID_POIGNET);
//   // DEBUG_SERIAL.print(theta_zero_poignet);

//   dxl.torqueOff(ID_POIGNET);
//   dxl.writeControlTableItem(HOMING_OFFSET,ID_POIGNET,-theta_zero_poignet);
//   dxl.torqueOn(ID_POIGNET);

}

void loop() {

  DEBUG_SERIAL.print("THETA EPAULE: ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(ID_EPAULE, UNIT_DEGREE));
  DEBUG_SERIAL.print(" THETA COUDE: ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE));
  DEBUG_SERIAL.print(" THETA POIGNET: ");
  DEBUG_SERIAL.println(dxl.getPresentPosition(ID_POIGNET, UNIT_DEGREE));

  // DEBUG_SERIAL.print(" PWM EPAULE ");
  // DEBUG_SERIAL.print(dxl.getPresentPWM(ID_EPAULE));
  // DEBUG_SERIAL.print(" PWM COUDE ");
  // DEBUG_SERIAL.println(dxl.getPresentPWM(ID_COUDE));
 
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
  int N_moyenne = 10;
  float PWM_epaule = 0.0;
  float PWM_coude = 0.0;
  float PWM_poignet = 0.0;
  
  set_mode(OP_EXTENDED_POSITION);
  dxl.setGoalPWM(ID_EPAULE, 500);
  dxl.setGoalPWM(ID_COUDE, 500);
  dxl.setGoalPWM(ID_POIGNET, 500);

  dxl.setGoalPosition(ID_EPAULE, zero_offset_epaule, UNIT_DEGREE);//VALEUR POUR 90 deg TODO: REDEFINIR 0 COMME 90 DEG
  dxl.setGoalPosition(ID_COUDE, zero_offset_coude, UNIT_DEGREE);//VALEUR POUR 90 deg 
  dxl.setGoalPosition(ID_POIGNET, zero_offset_poignet, UNIT_DEGREE);//VALEUR POUR 90 deg 
  delay(1000);

  for (int i = 0; i<N_moyenne ; i++)
  {
    PWM_epaule += dxl.getPresentPWM(ID_EPAULE);
    PWM_coude += dxl.getPresentPWM(ID_COUDE);
    PWM_poignet += dxl.getPresentPWM(ID_POIGNET);
    //DEBUG_SERIAL.println(dxl.getPresentPWM(ID_EPAULE));
    delay(200);
  }
  max_PWM_epaule = abs(PWM_epaule/N_moyenne);
  max_PWM_coude = abs(PWM_coude/N_moyenne);
  max_PWM_poignet = abs(PWM_poignet/N_moyenne);
}

void anti_gravite(){
  set_mode(OP_VELOCITY);

  dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule+2);
  dxl.setGoalVelocity(ID_EPAULE, 0);

  dxl.setGoalPWM(ID_COUDE, max_PWM_coude+2);
  dxl.setGoalVelocity(ID_COUDE, 0);

  dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet+2);
  dxl.setGoalVelocity(ID_POIGNET, 0);
}

void set_PosGoal_deg(const uint8_t ID, float goal){
  if(ID == ID_EPAULE)
  {
    if(goal > 90.0)
      goal = 90.0;

    if(goal < -90.0)
      goal = -90.0;
    goal + zero_offset_epaule;
  }
  
  if(ID == ID_COUDE)
  {
    if(goal > 110.0)//TODO: JSP L'ANGLE À VERIF
      goal = 110.0;

    if(goal < -1.0)
      goal = -1.0;
    goal + zero_offset_coude;
  }

  if(ID == ID_POIGNET)
  {
    if(goal > 75.0 + zero_offset_poignet)//TODO: JSP L'ANGLE À VERIF
      goal = 75.0;

    if(goal < -75.0 + zero_offset_poignet)//TODO: JSP L'ANGLE À VERIF
      goal = -75.0;
    goal + zero_offset_poignet;
  }
  dxl.setGoalPosition(ID, goal, UNIT_DEGREE);
}
