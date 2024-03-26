// Arduino MEGA projet S4
// Équipe 6 - GRO68 H24



///////////////////////////
// ***    Headers    *** //
///////////////////////////
#include <RTOS.h>
#include <Dynamixel2Arduino.h>

///////////////////////////
//   Variables globales  //
///////////////////////////

// Données de calcul
struct joint
{
  float angle;
  float torque;
  float goalCourant;
  float goalTension;
  float commandeMoteur;
};

struct exoSquelette
{
  joint poignet;
  joint coude;
  joint epaule;
};

exoSquelette exo = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Structures FreeRTOS
SemaphoreHandle_t mutex_data;

//TODO: VOIR CHANGEMENT ET IMPLEMENTER!
// JE VEUS UN ENUM QUI RELI CA À DES TRUCS CONCRET:
enum TypeDeMode{
  E_STOP = 0,
  MANUEL = 1,
  ANTI_GRATIVE = 2,
  CALIBRATION = 3,
};
uint16_t runmode = E_STOP; // = 0 : E-stop
                 // = 1 : contrôle manuel de l'interface
                 // = 2 : contrôle automatique anti-gravité
                 // = 3 : calibration

//++++++++++++++++++++++++++++++++++++++++++++++VARIABLE POUR MOTEURS++++++++++++++++++++++++++++++++++++++++++++++
// PARAMS SPÉCIFIQUE À OPENCR
#define DXL_SERIAL   Serial3
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 84; // OpenCR Board's DIR PIN.
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
const float DXL_PROTOCOL_VERSION = 2.0;

const uint8_t ID_EPAULE = 1;
const uint8_t ID_COUDE = 2;
const uint8_t ID_POIGNET = 6;
//This namespace is required to use Control table item names
using namespace ControlTableItem;

//PAS BESOIN DE MUTEXT POUR SES VARIABLES. SEULEMENT UTILISER DANS UNE TÂCHE AUCUNE CONCURRENCE POSSIBLE.
double max_PWM_epaule = 0.0;
double max_PWM_coude = 0.0;
double max_PWM_poignet = 0.0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void taskCommInterface(void *pvParameters);
void taskCalculTorque(void *pvParameters);

osThreadId thread_id_interface;
osThreadId thread_id_test;
osThreadId thread_id_moteurs_controls;

void setup()
{
  Serial.begin(115200);

  //++++++++++++++++SET_UP POUR MOTEUR++++++++++++++++
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(ID_EPAULE);
  dxl.ping(ID_COUDE);
  dxl.ping(ID_POIGNET);

  dxl.torqueOff(ID_EPAULE);
  dxl.setOperatingMode(ID_EPAULE, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_EPAULE);

  dxl.torqueOff(ID_COUDE);
  dxl.setOperatingMode(ID_COUDE, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_COUDE);  

  dxl.torqueOff(ID_POIGNET);
  dxl.setOperatingMode(ID_POIGNET, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_POIGNET);

  //WIRE TABLE
  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 30);
  // TODO: À TESTER:
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_EPAULE, 30);// TODO: VOIR SI CA MARCHE LOL
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_COUDE, 30);
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_POIGNET, 30);

  //++++++++++++++++++++++++++++++++++++++++++++++++++

  mutex_data = xSemaphoreCreateMutex();
  osThreadDef(interface, taskCommInterface, osPriorityNormal,0,2056);
  osThreadDef(ttestt, test, osPriorityNormal,0,2056);//changer test pour machine état moteurs
  osThreadDef(moving_moteurs, moteurs_controls, osPriorityNormal,0,2056);//changer test pour machine état moteurs

  thread_id_moteurs_controls = osThreadCreate(osThread(moving_moteurs), NULL);
  thread_id_interface = osThreadCreate(osThread(interface), NULL);
  thread_id_test = osThreadCreate(osThread(ttestt), NULL);

  osKernelStart();
  //SI JAMAIS ON A BESOIN
  //xTaskCreate(test,      "comm openrb",128,NULL,1,NULL);
  // xTaskCreate(taskCalculTorque, "calcul",     2056,NULL,0,NULL); 
}

void loop()
{  
}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void test( void const *pvParameters)
{
  (void) pvParameters;

  uint32_t start;
  uint32_t end;

  for(;;)
  {
    start=osKernelSysTick();

    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo.poignet.angle = exo.poignet.angle + 1;
      exo.poignet.torque = exo.poignet.torque + 2;
      exo.coude.angle = exo.coude.angle + 3;
      exo.coude.torque = exo.coude.torque + 4;
      exo.epaule.angle = exo.epaule.angle + 5;
      exo.epaule.torque = exo.epaule.torque + 6;
      xSemaphoreGive(mutex_data);
    }

    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(1000) - (end-start));
  }
}

void moteurs_controls( void const *pvParameters)
{
  (void) pvParameters;
  uint32_t start;
  uint32_t end;

  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t runmode_temp;
  for(;;)
  {
    start=osKernelSysTick();
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }

    switch(runmode_temp)
    {
      case E_STOP:
        //THINGS
      break;

      case ANTI_GRATIVE:
        calibration();
        anti_gravite();
      break;

      case MANUEL:
        set_mode(OP_EXTENDED_POSITION);
        set_PosGoal_deg(ID_COUDE, exo_temp.coude.commandeMoteur);
        set_PosGoal_deg(ID_EPAULE, exo_temp.epaule.commandeMoteur);
        set_PosGoal_deg(ID_POIGNET, exo_temp.poignet.commandeMoteur);
      break;

      default:
        //APPEL MEME FONCTION QUE E-STOP(I GUESS?)
      break;
    }

    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(100) - (end-start));
  }
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

  set_PosGoal_deg(ID_EPAULE, -97);//VALEUR POUR 90 deg TODO: REDEFINIR 0 COMME 90 DEG
  set_PosGoal_deg(ID_COUDE, 0);//VALEUR POUR 90 deg 
  set_PosGoal_deg(ID_POIGNET, 0);//VALEUR POUR 90 deg 
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

  dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule+5);
  dxl.setGoalVelocity(ID_EPAULE, 0);

  dxl.setGoalPWM(ID_COUDE, max_PWM_coude+5);
  dxl.setGoalVelocity(ID_COUDE, 0);

  dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet+5);
  dxl.setGoalVelocity(ID_POIGNET, 0);
}

void set_PosGoal_deg(const uint8_t ID, float goal){

  if(ID == ID_EPAULE)
  {
    if(goal > 90.0)
      goal = 90.0;

    if(goal < -90.0)
      goal = -90.0;
  }
  
  if(ID == ID_COUDE)
  {
    if(goal > 110.0)//TODO: JSP L'ANGLE À VERIF
      goal = 110.0;

    if(goal < -1.0)
      goal = -1.0;
  }

  if(ID == ID_POIGNET)
  {
    if(goal > 75.0)//TODO: JSP L'ANGLE À VERIF
      goal = 75.0;

    if(goal < -75.0)//TODO: JSP L'ANGLE À VERIF
      goal = -75.0;
  }
  dxl.setGoalPosition(ID, goal, UNIT_DEGREE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void taskCommInterface(void const *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t runmode_temp = 0;
  String message = "";
  String commandeActuel = "";
  int moteurActuel = 0;
  
  uint32_t start;
  uint32_t end;
  for(;;)
  {
    start=osKernelSysTick();

    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }

    ////// Envoi à l'interface //////
    Serial.println(String(millis()) + "," + String(exo_temp.poignet.angle) + "," + String(exo_temp.coude.angle) + "," + String(exo_temp.epaule.angle) + "," +
                   String(exo_temp.poignet.torque) + "," + String(exo_temp.coude.torque) + "," + String(exo_temp.epaule.torque));
    

    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');
      
      //Défini le runmode en fonction du message envoyé par l'interface. Format: b'(mode),commandePoignet,commandeCoude,commandeEpaule
      if(message.charAt(2) == '0')
        runmode_temp = 0;
      else if(message.charAt(2) == '1')
        runmode_temp = 1;
     else if(message.charAt(2) == '2')
        runmode_temp = 2;

      // Si mode manuel activé, traverse chaque lettre du message afin de lire la commande. 
      if (runmode_temp == 1)
      {
        for (int i = 4; i < message.length(); i++)
        {
          if(message.charAt(i) == ',')
          {
            if(moteurActuel==0)
            {
              exo_temp.poignet.commandeMoteur = commandeActuel.toFloat();
            }
            else if(moteurActuel==1)
              exo_temp.coude.commandeMoteur = commandeActuel.toFloat();
            else
              exo_temp.epaule.commandeMoteur = commandeActuel.toFloat();
            
            //Lorsqu'on croise une virgule, on a trouvé la fin de la commande. On doit donc la reset et changer de moteur
            moteurActuel++;
            commandeActuel = "";
          }
          //Ajoute chaque caractère dans commandeActuel, SAUF si la caractère est une virgule
          else 
            commandeActuel = commandeActuel + message.charAt(i);
        }
      }
    }
      
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo = exo_temp;
      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    }

    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(1000) - (end-start));
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Tâche permettant de calculer les valeurs de torque théoriquep..
// void taskCalculTorque(void *pvParameters)
// {
//   (void) pvParameters;

//   // Variables temporaires
//   exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//   uint16_t runmode_temp = 2;

//   // Constantes physiques
//   const float cstPoignet = 0.08193312;
//   const float cstCoude = 0.4133795679;
//   const float cstEpaule = 0.5955808941;

//   const float kt_gros = 1.8;
//   const float io_gros = 0.142;
//   const float kt_petit = 0.897;
//   const float io_petit = 0.131;
//   const float r_moteur = 1; // TO DO : à définir

//   for( ;; )
//   {
//     // Receive global variables
//     if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
//     {
//       exo_temp = exo;
//       runmode_temp = runmode;
//       xSemaphoreGive(mutex_data);
//     }

//     if(runmode_temp == 0) // E-stop
//     {
//       exo_temp.epaule.commandeMoteur = 0;
//       exo_temp.coude.commandeMoteur = 0;
//       exo_temp.poignet.commandeMoteur = 0;
//     }
//     else if(runmode_temp == 1) // Interface gère les commandes
//     {
//       // do nothing
//     }
//     else if(runmode_temp == 2) // Mode anti-gravité
//     {
//       exo_temp.poignet.torque = sin(exo_temp.poignet.angle) * cstPoignet;
//       exo_temp.coude.torque = exo_temp.poignet.torque + sin(exo_temp.coude.angle) * cstCoude;
//       exo_temp.epaule.torque = exo_temp.coude.torque + sin(exo_temp.epaule.angle) * cstEpaule;

//       exo_temp.poignet.goalCourant = (exo_temp.poignet.torque + kt_petit*io_petit)/io_petit;
//       exo_temp.coude.goalCourant = (exo_temp.coude.torque + kt_petit*io_petit)/io_petit;
//       exo_temp.epaule.goalCourant = (exo_temp.epaule.torque + kt_gros*io_gros)/io_gros;

//       exo_temp.poignet.goalTension = exo_temp.poignet.goalCourant/r_moteur;
//       exo_temp.coude.goalTension = exo_temp.coude.goalCourant/r_moteur;
      
//       exo_temp.poignet.commandeMoteur = exo_temp.poignet.goalTension;
//       exo_temp.coude.commandeMoteur = exo_temp.coude.goalTension;
//       exo_temp.epaule.commandeMoteur = exo_temp.epaule.goalCourant;
      
//     }
//     else if(runmode_temp == 3)
//     {
//       //autre méthode de controle!
//     }
  
//     // Send global variables
//     if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
//     {
//       exo = exo_temp;
//       runmode = runmode_temp;
//       xSemaphoreGive(mutex_data);
//     }
//   }
// }
