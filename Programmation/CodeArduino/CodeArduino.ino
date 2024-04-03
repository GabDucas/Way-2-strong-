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
  float velocite;
  float goalPWM;
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
  STATIQUE = 4
};
uint16_t runmode = E_STOP; // = 0 : E-stop
                 // = 1 : contrôle manuel de l'interface
                 // = 2 : contrôle automatique anti-gravité
                 // = 3 : calibration
                 // = 4 : statique à la position actuelle

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

float zero_offset_epaule = -5;
float zero_offset_coude = 27.19;// 20;
float zero_offset_poignet = 0;//355.62;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void taskCommInterface(void *pvParameters);
void taskCalculTorque(void *pvParameters);

osThreadId thread_id_interface;
osThreadId thread_id_update;
osThreadId thread_id_moteurs_controls;

void setup()
{
  Serial.begin(9600);

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

  dxl.writeControlTableItem(DRIVE_MODE, ID_POIGNET, 0);
  dxl.writeControlTableItem(DRIVE_MODE, ID_COUDE, 0);
  dxl.writeControlTableItem(DRIVE_MODE, ID_EPAULE, 0);


  //++++++++++++++++++++++++++++++++++++++++++++++++++

  mutex_data = xSemaphoreCreateMutex();
   exo = updateExo(exo);
   if(exo.poignet.angle > 300)
     zero_offset_poignet = 360;
  osThreadDef(interface, taskCommInterface, osPriorityNormal,0,2056);
  // osThreadDef(update, updateExo, osPriorityBelowNormal,0,2056);
  //osThreadDef(ttestt, test, osPriorityNormal,0,2056);//changer test pour machine état moteurs
  osThreadDef(moving_moteurs, moteurs_controls, osPriorityAboveNormal,0,2056);//changer test pour machine état moteurs PRIORITÉ BASSE 

  thread_id_moteurs_controls = osThreadCreate(osThread(moving_moteurs), NULL);
  thread_id_interface = osThreadCreate(osThread(interface), NULL);
  // thread_id_update = osThreadCreate(osThread(update), NULL);
  //thread_id_test = osThreadCreate(osThread(ttestt), NULL);

  osKernelStart();
  //SI JAMAIS ON A BESOIN
  //xTaskCreate(test,      "comm openrb",1528,NULL,15,NULL);
  // xTaskCreate(taskCalculTorque, "calcul",     2056,NULL,0,NULL); 
}

void loop()
{  
}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void moteurs_controls( void const *pvParameters)
{
  (void) pvParameters;
  uint32_t start;
  uint32_t end;

  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t runmode_temp;
  uint16_t runmode_temp_prev = 24;

  float commande_prev_epaule = 15000.0;
  float commande_prev_coude = 15000.0;
  float commande_prev_poignet = 15000.0;

  for(;;)
  {
    start=osKernelSysTick();
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo_temp = exo;
      runmode_temp = runmode;
      // Serial.println(runmode_temp);
      xSemaphoreGive(mutex_data);
    }
    // runmode_temp = ANTI_GRATIVE;
    if(runmode_temp != runmode_temp_prev || runmode_temp == MANUEL)
    {
      switch(runmode_temp)
      {
        case E_STOP:
          //THINGS
        break;

        case MANUEL:
          if(exo_temp.epaule.commandeMoteur != commande_prev_epaule || exo_temp.coude.commandeMoteur != commande_prev_coude || exo_temp.poignet.commandeMoteur != commande_prev_poignet )
          {
            set_mode(OP_EXTENDED_POSITION);
            set_PosGoal_deg(ID_COUDE, -exo_temp.coude.commandeMoteur + zero_offset_coude);
            set_PosGoal_deg(ID_EPAULE, -exo_temp.epaule.commandeMoteur + zero_offset_epaule);
            set_PosGoal_deg(ID_POIGNET, -exo_temp.poignet.commandeMoteur + zero_offset_poignet);
          }
        break;
        
        case ANTI_GRATIVE:
          calibration();
          anti_gravite();
        break;


        case STATIQUE:
          set_mode(OP_EXTENDED_POSITION);
          dxl.setGoalPosition(ID_COUDE, dxl.getPresentPosition(ID_COUDE));
          dxl.setGoalPosition(ID_EPAULE, dxl.getPresentPosition(ID_EPAULE));
          dxl.setGoalPosition(ID_POIGNET, dxl.getPresentPosition(ID_POIGNET));
        break;

        default:
          //APPEL MEME FONCTION QUE E-STOP(I GUESS?)
          //Appelle rien je penses
        break;
      }
    }

    // Serial.print("loop MODE: ");
    // Serial.println(runmode_temp);
    runmode_temp_prev = runmode_temp;
    commande_prev_epaule = exo_temp.epaule.commandeMoteur;
    commande_prev_coude = exo_temp.coude.commandeMoteur;
    commande_prev_poignet = exo_temp.poignet.commandeMoteur;
   
    
    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(500));
  }
}

void set_mode(int mode){
  // NICE TO HAVE:
  // PT FAUT TYPE CAST
  if ((int)dxl.readControlTableItem(OPERATING_MODE,ID_EPAULE) == mode){
    return;
  }
  else{
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

  set_PosGoal_deg(ID_EPAULE, zero_offset_epaule);//VALEUR POUR 90 deg TODO: REDEFINIR 0 COMME 90 DEG
  set_PosGoal_deg(ID_COUDE, zero_offset_coude);//VALEUR POUR 90 deg 
  set_PosGoal_deg(ID_POIGNET, zero_offset_poignet);//VALEUR POUR 90 deg 
  delay(500);

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

  if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
  {
    exo.poignet.goalPWM = max_PWM_poignet;
    exo.coude.goalPWM = max_PWM_coude;
    exo.epaule.goalPWM = max_PWM_epaule;
    xSemaphoreGive(mutex_data);
  }
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
  // if(ID == ID_EPAULE)
  // {
  //   if(goal > 90.0 + zero_offset_epaule)
  //     goal = 90.0;

  //   if(goal < -90.0 + zero_offset_epaule)
  //     goal = -90.0;
  // }
  
  // if(ID == ID_COUDE)
  // {
  //   if(goal > 15150.0 + zero_offset_coude)//TODO: JSP L'ANGLE À VERIF
  //     goal = 15150.0;

  //   if(goal < -15.0 + zero_offset_coude)
  //     goal = -15.0;
  // }

  // if(ID == ID_POIGNET)
  // {
  //   if(goal > 75.0 + zero_offset_poignet)//TODO: JSP L'ANGLE À VERIF
  //     goal = 75.0;

  //   if(goal < -75.0 + zero_offset_poignet)//TODO: JSP L'ANGLE À VERIF
  //     goal = -75.0;
  // }
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
    Serial.println( String(millis()) + "," + String(exo_temp.poignet.angle) + "," + String(exo_temp.coude.angle) + "," + String(exo_temp.epaule.angle) + "," +
                   String(exo_temp.poignet.torque) + "," + String(exo_temp.coude.torque) + "," + String(exo_temp.epaule.torque) + "," + String(exo_temp.poignet.velocite) +
                   "," + String(exo_temp.coude.velocite) + "," + String(exo_temp.epaule.velocite) + "," + String(exo_temp.poignet.goalPWM) +
                   "," + String(exo_temp.coude.goalPWM) + "," + String(exo_temp.epaule.goalPWM) );
    
    exo_temp = updateExo(exo_temp);
    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');

      //Défini le runmode en fonction du message envoyé par l'interface. Format: b'(mode),commandePoignet,commandeCoude,commandeEpaule
      if(message.charAt(0) == '0')
        runmode_temp = 0;
      else if(message.charAt(0) == '1')
        runmode_temp = 1;
     else if(message.charAt(0) == '2')
        runmode_temp = 2;

      // Si mode manuel activé, traverse chaque lettre du message afin de lire la commande. 
      if (runmode_temp == 1)
      {
        //  Serial.println(message);
        for (int i = 2; i < message.length(); i++)
        {
          if(message.charAt(i) == ',')
          {
              // Serial.println(moteurActuel);
            if(moteurActuel==0)
            {
              exo_temp.poignet.commandeMoteur = commandeActuel.toFloat();
              moteurActuel++;
              //  Serial.println(exo_temp.poignet.commandeMoteur);
            }
            else if(moteurActuel==1)
            {
              exo_temp.coude.commandeMoteur = commandeActuel.toFloat();
              moteurActuel++;
              //  Serial.println(exo_temp.coude.commandeMoteur);             
            }
            else if(moteurActuel == 2)
            {
              exo_temp.epaule.commandeMoteur = commandeActuel.toFloat();
              //  Serial.println(exo_temp.epaule.commandeMoteur);
              moteurActuel = 0;
            }
            //Lorsqu'on croise une virgule, on a trouvé la fin de la commande. On doit donc la reset et changer de moteur
            commandeActuel = "";
          }
          //Ajoute chaque caractère dans commandeActuel, SAUF si la caractère est une virgule
          else 
            commandeActuel = commandeActuel + message.charAt(i);
            // Serial.println(commandeActuel);
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
    osDelay(pdMS_TO_TICKS(150));
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Calcul le torque, récupères les différentes données des moteurs et update la struct de l'Exo
exoSquelette updateExo(exoSquelette exo_temp)
{

  // Variables temporaires
// exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Constantes physiques
const float cstPoignet = 0.081559331552;
const float cstCoude = 0.41533795679;
const float cstEpaule = 0.59558089415;

const float kt_gros = 15.8;
const float io_gros = 0.1542;
const float kt_petit = 0.897;
const float io_petit = 0.15315;
const float r_moteur = 15; // TO DO : à définir

// if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
//   {
//     exo_temp = exo;
//     xSemaphoreGive(mutex_data);
//   }
  // Lecture des données actuelles des moteurs
  // Lecture du goalPWM fait dans la calibration
  exo_temp.poignet.angle = (dxl.getPresentPosition(ID_POIGNET, UNIT_DEGREE) - zero_offset_poignet);//*3.15415592/180;
  exo_temp.coude.angle = (dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE) - zero_offset_coude);//*3.15415592/180;
  exo_temp.epaule.angle = (dxl.getPresentPosition(ID_EPAULE, UNIT_DEGREE) - zero_offset_epaule);//*3.15415592/180;


  exo_temp.poignet.velocite = dxl.getPresentVelocity(ID_POIGNET, UNIT_RPM);
  exo_temp.coude.velocite = dxl.getPresentVelocity(ID_COUDE, UNIT_RPM);
  exo_temp.epaule.velocite = dxl.getPresentVelocity(ID_EPAULE, UNIT_RPM);
  

  exo_temp.poignet.torque = sin(exo_temp.poignet.angle) * cstPoignet;
  exo_temp.coude.torque = exo_temp.poignet.torque + sin(exo_temp.coude.angle) * cstCoude;
  exo_temp.epaule.torque = exo_temp.coude.torque + sin(exo_temp.epaule.angle) * cstEpaule;


  //Send global variables
  // if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
  // {
  //   exo = exo_temp;
  //   xSemaphoreGive(mutex_data);
  // }
  // osDelay(pdMS_TO_TICKS(50));
  return exo_temp;
 
}
